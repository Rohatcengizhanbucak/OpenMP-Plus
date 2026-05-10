#include <SAMP+/client/COverlayRenderer.h>

#include <SAMP+/client/CGraphics.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Proxy/CMessageProxy.h>

#include <Detours/detours.h>

#include <imgui.h>
#include <backends/imgui_impl_dx9.h>
#include <backends/imgui_impl_win32.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
	typedef HRESULT(WINAPI* Reset_t)(IDirect3DDevice9*, D3DPRESENT_PARAMETERS*);
	typedef HRESULT(WINAPI* EndScene_t)(IDirect3DDevice9*);

	static const uintptr_t GtaSaD3DDeviceAddress = 0x00C97C28;
	static const unsigned int ResetVTableIndex = 16;
	static const unsigned int EndSceneVTableIndex = 42;

	static bool g_enabled = false;
	static bool g_hooksInstalled = false;
	static bool g_initialized = false;
	static bool g_failed = false;
	static IDirect3DDevice9* g_device = NULL;
	static HWND g_window = NULL;
	static Reset_t g_originalReset = NULL;
	static EndScene_t g_originalEndScene = NULL;

	static bool IsReadableAddress(const void* ptr, size_t bytes)
	{
		if (!ptr || !bytes)
			return false;

		MEMORY_BASIC_INFORMATION mbi = {};
		if (!VirtualQuery(ptr, &mbi, sizeof(mbi)))
			return false;

		if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			return false;

		const uintptr_t start = reinterpret_cast<uintptr_t>(ptr);
		const uintptr_t end = start + bytes;
		const uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
		return end <= regionEnd;
	}

	static bool IsExecutableAddress(const void* ptr)
	{
		if (!ptr)
			return false;

		MEMORY_BASIC_INFORMATION mbi = {};
		if (!VirtualQuery(ptr, &mbi, sizeof(mbi)))
			return false;

		if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			return false;

		const DWORD executableMask = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
		return (mbi.Protect & executableMask) != 0;
	}

	static IDirect3DDevice9* ReadKnownGtaDevice()
	{
		IDirect3DDevice9* device = NULL;
		__try
		{
			device = *reinterpret_cast<IDirect3DDevice9**>(GtaSaD3DDeviceAddress);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			device = NULL;
		}
		return device;
	}

	static IDirect3DDevice9* ScanMainModuleForDevice()
	{
		HMODULE module = GetModuleHandle(NULL);
		if (!module)
			return NULL;

		uintptr_t imageStart = reinterpret_cast<uintptr_t>(module);
		uintptr_t imageEnd = imageStart + 0x01000000;

		__try
		{
			IMAGE_DOS_HEADER* dos = reinterpret_cast<IMAGE_DOS_HEADER*>(module);
			IMAGE_NT_HEADERS* nt = reinterpret_cast<IMAGE_NT_HEADERS*>(imageStart + dos->e_lfanew);
			if (dos->e_magic == IMAGE_DOS_SIGNATURE && nt->Signature == IMAGE_NT_SIGNATURE && nt->OptionalHeader.SizeOfImage)
				imageEnd = imageStart + nt->OptionalHeader.SizeOfImage;
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
			imageEnd = imageStart + 0x01000000;
		}

		uintptr_t cursor = imageStart;
		while (cursor < imageEnd)
		{
			MEMORY_BASIC_INFORMATION mbi = {};
			if (!VirtualQuery(reinterpret_cast<void*>(cursor), &mbi, sizeof(mbi)))
				break;

			uintptr_t regionStart = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
			uintptr_t regionEnd = regionStart + mbi.RegionSize;
			if (regionEnd > imageEnd)
				regionEnd = imageEnd;

			const bool readable = mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD));
			if (readable)
			{
				for (uintptr_t p = regionStart; p + sizeof(void*) <= regionEnd; p += sizeof(void*))
				{
					IDirect3DDevice9* candidate = NULL;
					__try
					{
						candidate = *reinterpret_cast<IDirect3DDevice9**>(p);
					}
					__except (EXCEPTION_EXECUTE_HANDLER)
					{
						candidate = NULL;
					}

					if (COverlayRenderer::ValidateDevice(candidate))
						return candidate;
				}
			}

			cursor = regionEnd;
		}

		return NULL;
	}

	static void ApplyOverlayStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowRounding = 6.0f;
		style.FrameRounding = 5.0f;
		style.PopupRounding = 6.0f;
		style.ScrollbarRounding = 4.0f;
		style.WindowBorderSize = 1.0f;
		style.FrameBorderSize = 0.0f;
		style.WindowPadding = ImVec2(10.0f, 9.0f);
		style.FramePadding = ImVec2(10.0f, 7.0f);
		style.ItemSpacing = ImVec2(7.0f, 6.0f);

		ImVec4* colors = style.Colors;
		colors[ImGuiCol_WindowBg] = ImVec4(0.01f, 0.012f, 0.014f, 0.96f);
		colors[ImGuiCol_Border] = ImVec4(0.92f, 0.94f, 0.92f, 0.42f);
		colors[ImGuiCol_Text] = ImVec4(0.91f, 1.00f, 0.98f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.58f, 0.58f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.025f, 0.035f, 0.040f, 0.94f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.21f, 0.21f, 0.98f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.30f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.20f, 0.21f, 0.21f, 0.98f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.30f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.42f, 0.43f, 0.43f, 1.00f);
	}

	static HRESULT WINAPI Hook_Reset(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* presentationParameters)
	{
		COverlayRenderer::InvalidateDeviceObjects();
		HRESULT result = g_originalReset(device, presentationParameters);
		if (SUCCEEDED(result))
			COverlayRenderer::RestoreDeviceObjects();
		return result;
	}

	static HRESULT WINAPI Hook_EndScene(IDirect3DDevice9* device)
	{
		COverlayRenderer::Render(device);
		return g_originalEndScene(device);
	}
}

void COverlayRenderer::Enable()
{
	g_enabled = true;
}

void COverlayRenderer::Disable()
{
	g_enabled = false;
}

bool COverlayRenderer::IsEnabled()
{
	return g_enabled && !g_failed;
}

bool COverlayRenderer::IsReady()
{
	return IsEnabled() && g_initialized && g_hooksInstalled;
}

bool COverlayRenderer::ValidateDevice(IDirect3DDevice9* device)
{
	if (!device || !IsReadableAddress(device, sizeof(void*)))
		return false;

	__try
	{
		void** vtable = *reinterpret_cast<void***>(device);
		if (!IsReadableAddress(vtable, sizeof(void*) * (EndSceneVTableIndex + 1)))
			return false;

		return IsExecutableAddress(vtable[ResetVTableIndex]) && IsExecutableAddress(vtable[EndSceneVTableIndex]);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}
}

IDirect3DDevice9* COverlayRenderer::FindGameDevice()
{
	IDirect3DDevice9* device = CGraphics::GetDevice();
	if (ValidateDevice(device))
		return device;

	device = ReadKnownGtaDevice();
	if (ValidateDevice(device))
		return device;

	device = ScanMainModuleForDevice();
	if (ValidateDevice(device))
		return device;

	return NULL;
}

bool COverlayRenderer::TryInstall()
{
	if (!IsEnabled() || g_hooksInstalled)
		return g_hooksInstalled;

	IDirect3DDevice9* device = FindGameDevice();
	if (!device)
		return false;

	void** vtable = NULL;
	__try
	{
		vtable = *reinterpret_cast<void***>(device);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return false;
	}

	g_originalReset = reinterpret_cast<Reset_t>(DetourFunction(reinterpret_cast<PBYTE>(vtable[ResetVTableIndex]), reinterpret_cast<PBYTE>(Hook_Reset)));
	g_originalEndScene = reinterpret_cast<EndScene_t>(DetourFunction(reinterpret_cast<PBYTE>(vtable[EndSceneVTableIndex]), reinterpret_cast<PBYTE>(Hook_EndScene)));

	if (!g_originalReset || !g_originalEndScene)
	{
		if (g_originalEndScene)
			DetourRemove(reinterpret_cast<PBYTE>(g_originalEndScene), reinterpret_cast<PBYTE>(Hook_EndScene));
		if (g_originalReset)
			DetourRemove(reinterpret_cast<PBYTE>(g_originalReset), reinterpret_cast<PBYTE>(Hook_Reset));

		g_originalReset = NULL;
		g_originalEndScene = NULL;
		g_failed = true;
		CLog::Write("ImGui target overlay hook failed; target UI disabled");
		return false;
	}

	g_hooksInstalled = true;
	g_device = device;
	CLog::Write("ImGui target overlay hook installed through existing D3D9 device");
	return true;
}

bool COverlayRenderer::EnsureInitialized(IDirect3DDevice9* device)
{
	if (!IsEnabled() || !ValidateDevice(device))
		return false;

	if (g_initialized && g_device == device)
		return true;

	if (g_initialized)
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		g_initialized = false;
	}

	D3DDEVICE_CREATION_PARAMETERS creationParameters = {};
	if (FAILED(device->GetCreationParameters(&creationParameters)) || !creationParameters.hFocusWindow)
	{
		CLog::Write("ImGui target overlay could not resolve game window");
		return false;
	}

	g_device = device;
	g_window = creationParameters.hFocusWindow;
	CGraphics::AttachDevice(device);
	CMessageProxy::Initialize(g_window);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = NULL;
	io.LogFilename = NULL;
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	ApplyOverlayStyle();

	if (!ImGui_ImplWin32_Init(g_window) || !ImGui_ImplDX9_Init(device))
	{
		CLog::Write("ImGui target overlay backend initialization failed");
		Shutdown();
		g_failed = true;
		return false;
	}

	g_initialized = true;
	CLog::Write("ImGui target overlay initialized");
	return true;
}

void COverlayRenderer::Render(IDirect3DDevice9* device)
{
	if (!IsEnabled())
		return;

	__try
	{
		if (!EnsureInitialized(device))
			return;

		CGraphics::AttachDevice(device);

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		CTargetManager::Process();
		ImGui::GetIO().MouseDrawCursor = false;
		CTargetManager::RenderImGui();

		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		g_failed = true;
		CLog::Write("ImGui target overlay crashed internally; target UI disabled safely");
	}
}

void COverlayRenderer::InvalidateDeviceObjects()
{
	if (g_initialized)
		ImGui_ImplDX9_InvalidateDeviceObjects();
}

void COverlayRenderer::RestoreDeviceObjects()
{
	if (g_initialized)
		ImGui_ImplDX9_CreateDeviceObjects();
}

bool COverlayRenderer::HandleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (!g_initialized)
		return false;

	ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam);

	switch (message)
	{
	case WM_SETCURSOR:
		return false;
	case WM_MOUSEMOVE:
	case WM_LBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MOUSEWHEEL:
		return CTargetManager::ShouldCaptureMouse();
	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
	case WM_CHAR:
		return CTargetManager::ShouldSuppressKeyboard();
	default:
		return false;
	}
}

void COverlayRenderer::Shutdown()
{
	if (g_initialized)
	{
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
		g_initialized = false;
	}

	CMessageProxy::Uninitialize();

	if (g_hooksInstalled)
	{
		if (g_originalEndScene)
			DetourRemove(reinterpret_cast<PBYTE>(g_originalEndScene), reinterpret_cast<PBYTE>(Hook_EndScene));
		if (g_originalReset)
			DetourRemove(reinterpret_cast<PBYTE>(g_originalReset), reinterpret_cast<PBYTE>(Hook_Reset));
		g_hooksInstalled = false;
	}

	g_originalEndScene = NULL;
	g_originalReset = NULL;
	g_device = NULL;
	g_window = NULL;
}
