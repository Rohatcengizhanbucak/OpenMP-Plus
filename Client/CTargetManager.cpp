#include <SAMP+/CRPC.h>
#include <SAMP+/OMPPlusProtocol.h>
#include <SAMP+/client/CGraphics.h>
#include <SAMP+/client/CKeyBinds.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/COverlayRenderer.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/Proxy/CMessageProxy.h>

#include <DirectX/dinput.h>
#include <imgui.h>

#include <algorithm>

bool CTargetManager::m_hasContext = false;
bool CTargetManager::m_menuOpen = false;
bool CTargetManager::m_cursorOwned = false;
bool CTargetManager::m_lastAltDown = false;
bool CTargetManager::m_lastLeftDown = false;
bool CTargetManager::m_lastEscapeDown = false;
bool CTargetManager::m_lastRightDown = false;
bool CTargetManager::m_virtualCursorInitialized = false;
float CTargetManager::m_virtualCursorX = 0.0f;
float CTargetManager::m_virtualCursorY = 0.0f;
LONG CTargetManager::m_virtualWheel = 0;
bool CTargetManager::m_mouseButtons[5] = {};
unsigned int CTargetManager::m_targetId = 0;
unsigned int CTargetManager::m_flags = 0;
unsigned long CTargetManager::m_expiresAt = 0;
unsigned long CTargetManager::m_lastContextTick = 0;
unsigned long CTargetManager::m_mouseSuppressUntil = 0;
unsigned long CTargetManager::m_keyboardReleaseUntil = 0;
std::string CTargetManager::m_title;
std::vector<sTargetOption> CTargetManager::m_options;
int CTargetManager::m_hoverIndex = -1;
POINT CTargetManager::m_cursor = { 0, 0 };
ID3DXFont* CTargetManager::m_font = NULL;

namespace
{
	const unsigned char MaxTargetOptions = 8;
	const float MenuWidth = 230.0f;
	const float HeaderHeight = 30.0f;
	const float RowHeight = 30.0f;
	const float RowGap = 4.0f;
	const unsigned long OpenContextGraceMs = 900;
	const unsigned long KeyboardReleaseMs = 80;
	const unsigned long PostMouseSuppressMs = 180;
	const float VirtualMouseSensitivity = 1.0f;
}

void CTargetManager::HandleSetContext(RakNet::BitStream& bitStream)
{
	unsigned int targetId = 0;
	unsigned short ttlMs = 0;
	unsigned int flags = 0;
	std::string title;
	unsigned char optionCount = 0;

	if (!bitStream.Read(targetId)
		|| !bitStream.Read(ttlMs)
		|| !bitStream.Read(flags)
		|| !ReadBoundString(bitStream, title, 48)
		|| !bitStream.Read(optionCount)
		|| targetId == 0
		|| optionCount > MaxTargetOptions)
	{
		return;
	}

	std::vector<sTargetOption> options;
	for (unsigned char i = 0; i < optionCount; ++i)
	{
		sTargetOption option;
		if (!bitStream.Read(option.optionId)
			|| !bitStream.Read(option.enabled)
			|| !ReadBoundString(bitStream, option.label, 48)
			|| !ReadBoundString(bitStream, option.icon, 24)
			|| option.optionId == 0)
		{
			return;
		}
		options.push_back(option);
	}

	if (ttlMs == 0)
		ttlMs = 500;
	if (ttlMs > 5000)
		ttlMs = 5000;

	m_hasContext = true;
	m_targetId = targetId;
	m_flags = flags;
	m_title = title;
	m_options = options;
	m_expiresAt = GetTickCount() + ttlMs;
	m_lastContextTick = GetTickCount();

	if (m_options.empty())
		ClearContext();
}

void CTargetManager::ClearContext()
{
	m_hasContext = false;
	m_targetId = 0;
	m_flags = 0;
	m_expiresAt = 0;
	m_title.clear();
	m_options.clear();
	m_hoverIndex = -1;
	m_lastContextTick = 0;
	CloseMenu(true);
}

void CTargetManager::Process()
{
	const bool altDown = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
	const bool escapeDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
	const bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;

	if (!HasValidContext())
	{
		if (m_hasContext)
			ClearContext();
		else
			CloseMenu(true);
		m_lastAltDown = altDown;
		m_lastEscapeDown = escapeDown;
		m_lastRightDown = rightDown;
		return;
	}

	if (!InputAllowed())
	{
		CloseMenu(true);
		m_lastAltDown = altDown;
		m_lastEscapeDown = escapeDown;
		m_lastRightDown = rightDown;
		m_lastLeftDown = false;
		return;
	}

	if (altDown && !m_lastAltDown)
	{
		if (m_menuOpen)
			CloseMenu(true);
		else
			OpenMenu();
	}
	else if (m_menuOpen && ((escapeDown && !m_lastEscapeDown) || (rightDown && !m_lastRightDown)))
	{
		CloseMenu(true);
	}

	m_lastAltDown = altDown;
	m_lastEscapeDown = escapeDown;
	m_lastRightDown = rightDown;

	if (!m_menuOpen)
	{
		m_lastLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		return;
	}

	if (COverlayRenderer::IsReady())
	{
		m_lastLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
		return;
	}

	UpdateHover();

	const bool leftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	if (leftDown && !m_lastLeftDown && m_hoverIndex >= 0 && m_hoverIndex < static_cast<int>(m_options.size()))
	{
		const sTargetOption& option = m_options[m_hoverIndex];
		if (option.enabled)
			SendSelect(option.optionId);
	}
	m_lastLeftDown = leftDown;
}

void CTargetManager::Draw(IDirect3DDevice9* device)
{
	if (!device || !HasValidContext())
		return;

	EnsureFont(device);

	CPoint2D& resolution = CGraphics::GetScreenResolution();
	const float cx = static_cast<float>(resolution.X()) * 0.5f;
	const float cy = static_cast<float>(resolution.Y()) * 0.5f;

	if (m_menuOpen)
		DrawMenu(device, cx, cy);
	else if (ShouldDrawPrompt())
		DrawEye(device, cx, cy);
}

void CTargetManager::OnDeviceInit(IDirect3DDevice9* device)
{
	EnsureFont(device);
}

void CTargetManager::OnLostDevice()
{
	if (m_font)
		m_font->OnLostDevice();
}

void CTargetManager::OnResetDevice()
{
	if (m_font)
		m_font->OnResetDevice();
}

void CTargetManager::CleanUp()
{
	CloseMenu(true);
	if (m_font)
	{
		m_font->Release();
		m_font = NULL;
	}
}

bool CTargetManager::IsMenuOpen()
{
	return m_menuOpen;
}

bool CTargetManager::HasActiveContext()
{
	return m_hasContext && m_targetId != 0 && !m_options.empty() && static_cast<long>(GetTickCount() - m_expiresAt) < 0;
}

bool CTargetManager::ShouldCaptureMouse()
{
	return (m_menuOpen || IsMouseSuppressed()) && InputAllowed();
}

bool CTargetManager::ShouldSuppressKeyboard()
{
	return IsKeyboardReleaseActive() && InputAllowed();
}

bool CTargetManager::ShouldNeutralizeKeyboard()
{
	return IsKeyboardReleaseActive() && InputAllowed();
}

bool CTargetManager::ShouldConsumeDirectInputOffset(DWORD offset)
{
	return ShouldConsumeDirectInputEvent(offset, 0x80);
}

bool CTargetManager::ShouldConsumeDirectInputEvent(DWORD offset, DWORD data)
{
	if (!InputAllowed())
		return false;

	const bool keyDown = (data & 0x80) != 0;
	if (IsKeyboardReleaseActive())
		return true;

	if (HasActiveContext())
		return keyDown && (offset == DIK_LMENU || offset == DIK_RMENU);

	return false;
}

void CTargetManager::FilterKeyboardState(DWORD size, LPVOID state)
{
	if (!state || !size)
		return;

	if (!InputAllowed())
		return;

	BYTE* keys = static_cast<BYTE*>(state);
	if (IsKeyboardReleaseActive())
	{
		memset(keys, 0, size);
		return;
	}

	if (HasActiveContext())
	{
		if (size > DIK_LMENU)
			keys[DIK_LMENU] = 0;
		if (size > DIK_RMENU)
			keys[DIK_RMENU] = 0;
	}
}

void CTargetManager::FilterMouseState(DWORD size, LPVOID state)
{
	if (!state || !size)
		return;

	if (!m_menuOpen && !IsMouseSuppressed())
		return;

	if (m_menuOpen)
	{
		if (size >= sizeof(DIMOUSESTATE2))
		{
			DIMOUSESTATE2* mouse = static_cast<DIMOUSESTATE2*>(state);
			AddMouseDelta(mouse->lX, mouse->lY, mouse->lZ);
			for (unsigned int i = 0; i < 5; ++i)
				SetMouseButton(i, (mouse->rgbButtons[i] & 0x80) != 0);
		}
		else if (size >= sizeof(DIMOUSESTATE))
		{
			DIMOUSESTATE* mouse = static_cast<DIMOUSESTATE*>(state);
			AddMouseDelta(mouse->lX, mouse->lY, mouse->lZ);
			for (unsigned int i = 0; i < 4; ++i)
				SetMouseButton(i, (mouse->rgbButtons[i] & 0x80) != 0);
		}
	}

	memset(state, 0, size);
}

void CTargetManager::AddMouseDelta(LONG x, LONG y, LONG wheel)
{
	if (!m_menuOpen)
		return;

	if (!m_virtualCursorInitialized)
		InitializeVirtualCursor();

	m_virtualCursorX += static_cast<float>(x) * VirtualMouseSensitivity;
	m_virtualCursorY += static_cast<float>(y) * VirtualMouseSensitivity;
	m_virtualWheel += wheel;
}

void CTargetManager::SetMouseButton(unsigned int button, bool down)
{
	if (button >= 5)
		return;
	if (!m_menuOpen)
		return;

	m_mouseButtons[button] = down;
}

void CTargetManager::RenderImGui()
{
	if (!HasValidContext() || !InputAllowed())
		return;

	ImGuiIO& io = ImGui::GetIO();
	ApplyImGuiInput();

	const float cx = io.DisplaySize.x * 0.5f;
	const float cy = io.DisplaySize.y * 0.5f;
	const ImU32 teal = IM_COL32(54, 235, 210, 230);
	const ImU32 tealSoft = IM_COL32(54, 235, 210, 90);

	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const bool drawPrompt = ShouldDrawPrompt();
	if (drawPrompt)
	{
		draw->AddCircle(ImVec2(cx, cy), 13.0f, tealSoft, 32, 2.0f);
		draw->AddCircleFilled(ImVec2(cx, cy), 4.0f, teal);
		draw->AddLine(ImVec2(cx - 30.0f, cy), ImVec2(cx - 18.0f, cy), teal, 1.5f);
		draw->AddLine(ImVec2(cx + 18.0f, cy), ImVec2(cx + 30.0f, cy), teal, 1.5f);
	}

	if (!m_menuOpen)
	{
		if (drawPrompt)
			draw->AddText(ImVec2(cx + 22.0f, cy - 9.0f), teal, "ALT");
		return;
	}

	const float width = MenuWidth;
	const float totalHeight = HeaderHeight + RowGap + (RowHeight + RowGap) * static_cast<float>(m_options.size()) + 10.0f;
	ImGui::SetNextWindowPos(ImVec2(cx + 30.0f, cy - totalHeight * 0.5f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, totalHeight), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.92f);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoScrollbar;

	unsigned int selectedOption = 0;

	ImGui::Begin("##ompp_target_menu", NULL, flags);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.91f, 1.00f, 0.98f, 1.00f));
	ImGui::TextUnformatted(m_title.empty() ? "Target" : m_title.c_str());
	ImGui::PopStyleColor();
	ImGui::Separator();

	for (size_t i = 0; i < m_options.size(); ++i)
	{
		const sTargetOption& option = m_options[i];
		std::string label = option.label;
		if (!option.icon.empty())
			label = option.icon + "  " + label;
		label += "##";
		label += std::to_string(option.optionId);

		if (!option.enabled)
			ImGui::BeginDisabled();

		if (ImGui::Button(label.c_str(), ImVec2(-1.0f, RowHeight)))
			selectedOption = option.optionId;
		if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			selectedOption = option.optionId;

		if (!option.enabled)
			ImGui::EndDisabled();
	}

	ImGui::End();

	if (selectedOption != 0)
		SendSelect(selectedOption);
}

bool CTargetManager::HasValidContext()
{
	if (!m_hasContext || m_targetId == 0 || m_options.empty())
		return false;

	const unsigned long now = GetTickCount();
	if (static_cast<long>(now - m_expiresAt) < 0)
		return true;

	return m_menuOpen && m_lastContextTick != 0 && (now - m_lastContextTick) <= OpenContextGraceMs;
}

bool CTargetManager::InputAllowed()
{
	HWND foreground = GetForegroundWindow();
	if (!foreground)
		return false;

	DWORD processId = 0;
	GetWindowThreadProcessId(foreground, &processId);
	if (processId != GetCurrentProcessId())
		return false;

	return !CKeyBinds::IsTextInputActive();
}

bool CTargetManager::IsMouseSuppressed()
{
	const unsigned long now = GetTickCount();
	if (m_mouseSuppressUntil != 0 && static_cast<long>(now - m_mouseSuppressUntil) < 0)
		return true;

	m_mouseSuppressUntil = 0;
	return false;
}

void CTargetManager::SuppressMouseInput(unsigned long durationMs)
{
	const unsigned long now = GetTickCount();
	m_mouseSuppressUntil = now + durationMs;
}

bool CTargetManager::IsKeyboardReleaseActive()
{
	const unsigned long now = GetTickCount();
	if (m_keyboardReleaseUntil != 0 && static_cast<long>(now - m_keyboardReleaseUntil) < 0)
		return true;

	m_keyboardReleaseUntil = 0;
	return false;
}

bool CTargetManager::ShouldDrawPrompt()
{
	return (m_flags & OMPPlusProtocol::TargetFlagHidePrompt) == 0;
}

void CTargetManager::OpenMenu()
{
	const unsigned long now = GetTickCount();
	m_mouseSuppressUntil = 0;
	m_keyboardReleaseUntil = now + KeyboardReleaseMs;
	m_menuOpen = true;
	m_hoverIndex = -1;
	InitializeVirtualCursor();
	if (!CGraphics::IsCursorEnabled())
	{
		CGraphics::ToggleCursor(true);
		m_cursorOwned = true;
	}
	SendMode(true);
}

void CTargetManager::CloseMenu(bool clearCursor)
{
	const bool wasOpen = m_menuOpen;
	if (m_menuOpen)
		SendMode(false);

	m_menuOpen = false;
	m_hoverIndex = -1;
	m_lastLeftDown = false;
	m_keyboardReleaseUntil = 0;
	m_virtualCursorInitialized = false;
	m_virtualWheel = 0;
	memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
	if (wasOpen)
	{
		SuppressMouseInput(PostMouseSuppressMs);
	}

	if (clearCursor && m_cursorOwned)
	{
		CGraphics::ToggleCursor(false);
		m_cursorOwned = false;
	}
}

void CTargetManager::InitializeVirtualCursor()
{
	CPoint2D& resolution = CGraphics::GetScreenResolution();
	float width = static_cast<float>(resolution.X());
	float height = static_cast<float>(resolution.Y());

	if (width <= 0.0f)
		width = 1280.0f;
	if (height <= 0.0f)
		height = 720.0f;

	const float cx = width * 0.5f;
	const float cy = height * 0.5f;
	const float totalHeight = HeaderHeight + RowGap + (RowHeight + RowGap) * static_cast<float>(m_options.size()) + 10.0f;
	const float menuX = cx + 30.0f;
	const float menuY = cy - totalHeight * 0.5f;

	m_virtualCursorX = menuX + MenuWidth * 0.5f;
	m_virtualCursorY = menuY + HeaderHeight + RowGap + RowHeight * 0.5f;
	m_virtualCursorInitialized = true;
}

void CTargetManager::ApplyImGuiInput()
{
	if (!m_virtualCursorInitialized)
		InitializeVirtualCursor();

	ImGuiIO& io = ImGui::GetIO();
	const float maxX = (std::max)(1.0f, io.DisplaySize.x - 1.0f);
	const float maxY = (std::max)(1.0f, io.DisplaySize.y - 1.0f);

	m_virtualCursorX = (std::max)(0.0f, (std::min)(m_virtualCursorX, maxX));
	m_virtualCursorY = (std::max)(0.0f, (std::min)(m_virtualCursorY, maxY));

	io.MousePos = ImVec2(m_virtualCursorX, m_virtualCursorY);
	for (int i = 0; i < 5; ++i)
		io.MouseDown[i] = m_menuOpen && m_mouseButtons[i];

	if (m_virtualWheel != 0)
	{
		io.MouseWheel += static_cast<float>(m_virtualWheel) / 120.0f;
		m_virtualWheel = 0;
	}
}

void CTargetManager::SendMode(bool opened)
{
	if (!Network::IsConnected() || m_targetId == 0)
		return;

	RakNet::BitStream bitStream;
	bitStream.Write(m_targetId);
	bitStream.Write(opened);
	Network::SendRPC(eRPC::ON_TARGET_MODE, &bitStream);
}

void CTargetManager::SendSelect(unsigned int optionId)
{
	if (!Network::IsConnected() || m_targetId == 0 || optionId == 0)
		return;

	RakNet::BitStream bitStream;
	bitStream.Write(m_targetId);
	bitStream.Write(optionId);
	Network::SendRPC(eRPC::ON_TARGET_SELECT, &bitStream);
	CloseMenu(true);
	ClearContext();
}

void CTargetManager::UpdateHover()
{
	HWND hwnd = CMessageProxy::GetWindowHandle();
	GetCursorPos(&m_cursor);
	if (hwnd)
		ScreenToClient(hwnd, &m_cursor);

	CPoint2D& resolution = CGraphics::GetScreenResolution();
	const float cx = static_cast<float>(resolution.X()) * 0.5f;
	const float cy = static_cast<float>(resolution.Y()) * 0.5f;
	const float x = cx + 28.0f;
	const float y = cy - (HeaderHeight + (RowHeight + RowGap) * static_cast<float>(m_options.size())) * 0.5f;

	m_hoverIndex = -1;
	for (size_t i = 0; i < m_options.size(); ++i)
	{
		const float rowY = y + HeaderHeight + RowGap + static_cast<float>(i) * (RowHeight + RowGap);
		if (m_cursor.x >= x && m_cursor.x <= x + MenuWidth && m_cursor.y >= rowY && m_cursor.y <= rowY + RowHeight)
		{
			m_hoverIndex = static_cast<int>(i);
			break;
		}
	}
}

bool CTargetManager::ReadBoundString(RakNet::BitStream& bitStream, std::string& value, unsigned char maxLength)
{
	unsigned char length = 0;
	if (!bitStream.Read(length) || length > maxLength)
		return false;

	char buffer[64] = {};
	if (length && !bitStream.Read(buffer, length))
		return false;

	value.assign(buffer, length);
	return true;
}

void CTargetManager::EnsureFont(IDirect3DDevice9* device)
{
	if (m_font || !device)
		return;

	D3DXCreateFontA(device, 16, 0, FW_MEDIUM, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial", &m_font);
}

void CTargetManager::DrawFilledRect(IDirect3DDevice9* device, float x, float y, float w, float h, D3DCOLOR colour)
{
	struct Vertex
	{
		float x, y, z, rhw;
		D3DCOLOR colour;
	};

	Vertex vertices[4] =
	{
		{ x, y, 0.0f, 1.0f, colour },
		{ x + w, y, 0.0f, 1.0f, colour },
		{ x, y + h, 0.0f, 1.0f, colour },
		{ x + w, y + h, 0.0f, 1.0f, colour }
	};

	device->SetTexture(0, NULL);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, vertices, sizeof(Vertex));
}

void CTargetManager::DrawOutlineRect(IDirect3DDevice9* device, float x, float y, float w, float h, D3DCOLOR colour)
{
	DrawLine(device, x, y, x + w, y, colour);
	DrawLine(device, x + w, y, x + w, y + h, colour);
	DrawLine(device, x + w, y + h, x, y + h, colour);
	DrawLine(device, x, y + h, x, y, colour);
}

void CTargetManager::DrawLine(IDirect3DDevice9* device, float x1, float y1, float x2, float y2, D3DCOLOR colour)
{
	struct Vertex
	{
		float x, y, z, rhw;
		D3DCOLOR colour;
	};

	Vertex vertices[2] =
	{
		{ x1, y1, 0.0f, 1.0f, colour },
		{ x2, y2, 0.0f, 1.0f, colour }
	};

	device->SetTexture(0, NULL);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
	device->DrawPrimitiveUP(D3DPT_LINELIST, 1, vertices, sizeof(Vertex));
}

void CTargetManager::DrawTextLine(const std::string& text, int x, int y, int w, int h, D3DCOLOR colour)
{
	if (!m_font)
		return;

	RECT rect = { x, y, x + w, y + h };
	m_font->DrawTextA(NULL, text.c_str(), -1, &rect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS, colour);
}

void CTargetManager::DrawEye(IDirect3DDevice9* device, float cx, float cy)
{
	const D3DCOLOR teal = D3DCOLOR_ARGB(230, 54, 235, 210);
	const D3DCOLOR tealDim = D3DCOLOR_ARGB(80, 54, 235, 210);

	DrawFilledRect(device, cx - 5.0f, cy - 5.0f, 10.0f, 10.0f, teal);
	DrawOutlineRect(device, cx - 16.0f, cy - 12.0f, 32.0f, 24.0f, tealDim);
	DrawLine(device, cx - 28.0f, cy, cx - 18.0f, cy, teal);
	DrawLine(device, cx + 18.0f, cy, cx + 28.0f, cy, teal);
	DrawTextLine("ALT", static_cast<int>(cx + 20.0f), static_cast<int>(cy - 10.0f), 60, 20, teal);
}

void CTargetManager::DrawMenu(IDirect3DDevice9* device, float cx, float cy)
{
	const float x = cx + 28.0f;
	const float totalHeight = HeaderHeight + RowGap + (RowHeight + RowGap) * static_cast<float>(m_options.size());
	const float y = cy - totalHeight * 0.5f;

	if (ShouldDrawPrompt())
		DrawEye(device, cx, cy);
	DrawFilledRect(device, x, y, MenuWidth, HeaderHeight, D3DCOLOR_ARGB(205, 20, 34, 38));
	DrawOutlineRect(device, x, y, MenuWidth, HeaderHeight, D3DCOLOR_ARGB(220, 54, 235, 210));
	DrawTextLine(m_title.empty() ? "Target" : m_title, static_cast<int>(x + 10.0f), static_cast<int>(y), static_cast<int>(MenuWidth - 20.0f), static_cast<int>(HeaderHeight), D3DCOLOR_ARGB(255, 232, 255, 252));

	for (size_t i = 0; i < m_options.size(); ++i)
	{
		const sTargetOption& option = m_options[i];
		const float rowY = y + HeaderHeight + RowGap + static_cast<float>(i) * (RowHeight + RowGap);
		const bool hovered = static_cast<int>(i) == m_hoverIndex;
		const D3DCOLOR bg = hovered ? D3DCOLOR_ARGB(230, 31, 72, 72) : D3DCOLOR_ARGB(205, 17, 28, 32);
		const D3DCOLOR border = hovered ? D3DCOLOR_ARGB(255, 54, 235, 210) : D3DCOLOR_ARGB(120, 54, 235, 210);
		const D3DCOLOR text = option.enabled ? D3DCOLOR_ARGB(255, 232, 255, 252) : D3DCOLOR_ARGB(150, 190, 202, 202);

		DrawFilledRect(device, x, rowY, MenuWidth, RowHeight, bg);
		DrawOutlineRect(device, x, rowY, MenuWidth, RowHeight, border);

		std::string label = option.label;
		if (!option.icon.empty())
			label = option.icon + "  " + label;
		DrawTextLine(label, static_cast<int>(x + 10.0f), static_cast<int>(rowY), static_cast<int>(MenuWidth - 20.0f), static_cast<int>(RowHeight), text);
	}
}
