#include <SAMP+/CRPC.h>
#include <SAMP+/OMPPlusProtocol.h>
#include <SAMP+/client/CBuildManager.h>
#include <SAMP+/client/CGraphics.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/Proxy/CDInput8DeviceProxy.h>
#include <SAMP+/client/Proxy/CMessageProxy.h>

#include <imgui.h>

#include <algorithm>
#include <cstring>
#include <cstdio>

bool CBuildManager::m_active = false;
bool CBuildManager::m_menuOpen = false;
bool CBuildManager::m_cursorOwned = false;
bool CBuildManager::m_virtualCursorInitialized = false;
bool CBuildManager::m_lastLeftDown = false;
bool CBuildManager::m_lastRightDown = false;
bool CBuildManager::m_lastMiddleDown = false;
bool CBuildManager::m_lastEscapeDown = false;
bool CBuildManager::m_lastQDown = false;
bool CBuildManager::m_lastEDown = false;
float CBuildManager::m_virtualCursorX = 0.0f;
float CBuildManager::m_virtualCursorY = 0.0f;
LONG CBuildManager::m_virtualWheel = 0;
bool CBuildManager::m_mouseButtons[5] = {};
unsigned int CBuildManager::m_sessionId = 0;
unsigned int CBuildManager::m_selectedPartId = 0;
float CBuildManager::m_maxDistance = 8.0f;
int CBuildManager::m_rotationStep = 0;
bool CBuildManager::m_flipped = false;
unsigned long CBuildManager::m_statusUntil = 0;
bool CBuildManager::m_statusSuccess = true;
unsigned long CBuildManager::m_inputGuardUntil = 0;
std::string CBuildManager::m_title;
std::string CBuildManager::m_status;
std::vector<sBuildPart> CBuildManager::m_parts;
float CBuildManager::m_menuX = 0.0f;
float CBuildManager::m_menuY = 0.0f;
float CBuildManager::m_menuW = 0.0f;
float CBuildManager::m_menuH = 0.0f;

namespace
{
	struct sBuildStateLock
	{
		CRITICAL_SECTION section;

		sBuildStateLock()
		{
			InitializeCriticalSection(&section);
		}

		~sBuildStateLock()
		{
			DeleteCriticalSection(&section);
		}
	};

	class cScopedBuildStateLock
	{
	public:
		cScopedBuildStateLock()
		{
			EnterCriticalSection(&BuildLock().section);
		}

		~cScopedBuildStateLock()
		{
			LeaveCriticalSection(&BuildLock().section);
		}

	private:
		static sBuildStateLock& BuildLock()
		{
			static sBuildStateLock lock;
			return lock;
		}
	};

	struct sKeyboardReleaseKey
	{
		DWORD offset;
		int virtualKey;
	};

	const sKeyboardReleaseKey KeyboardReleaseKeys[] =
	{
		{ DIK_D, 'D' },
		{ DIK_A, 'A' },
		{ DIK_W, 'W' },
		{ DIK_S, 'S' },
		{ DIK_RIGHT, VK_RIGHT },
		{ DIK_LEFT, VK_LEFT },
		{ DIK_UP, VK_UP },
		{ DIK_DOWN, VK_DOWN },
		{ DIK_NUMPAD6, VK_NUMPAD6 },
		{ DIK_NUMPAD4, VK_NUMPAD4 },
		{ DIK_NUMPAD8, VK_NUMPAD8 },
		{ DIK_NUMPAD2, VK_NUMPAD2 },
		{ DIK_LMENU, VK_LMENU },
		{ DIK_RMENU, VK_RMENU },
		{ DIK_LSHIFT, VK_LSHIFT },
		{ DIK_RSHIFT, VK_RSHIFT },
		{ DIK_LCONTROL, VK_LCONTROL },
		{ DIK_RCONTROL, VK_RCONTROL },
		{ DIK_SPACE, VK_SPACE },
		{ DIK_RETURN, VK_RETURN },
		{ DIK_LBRACKET, VK_OEM_4 },
		{ DIK_RBRACKET, VK_OEM_6 },
		{ DIK_Q, 'Q' },
		{ DIK_E, 'E' }
	};

	bool IsPhysicalKeyDown(int virtualKey)
	{
		return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
	}

	float ClampFloat(float value, float minValue, float maxValue)
	{
		return (std::max)(minValue, (std::min)(value, maxValue));
	}

	ImVec2 GetOverlayDisplaySize()
	{
		ImGuiIO& io = ImGui::GetIO();
		float width = io.DisplaySize.x;
		float height = io.DisplaySize.y;

		if (width <= 1.0f || height <= 1.0f)
		{
			CPoint2D& resolution = CGraphics::GetScreenResolution();
			width = static_cast<float>(resolution.X());
			height = static_cast<float>(resolution.Y());
		}

		if (width <= 1.0f)
			width = 1280.0f;
		if (height <= 1.0f)
			height = 720.0f;

		return ImVec2(width, height);
	}

	ImVec2 ScaleClientPointToDisplay(HWND window, const POINT& point)
	{
		ImVec2 scaled(static_cast<float>(point.x), static_cast<float>(point.y));
		if (!window)
			return scaled;

		RECT client = {};
		if (!GetClientRect(window, &client))
			return scaled;

		const float clientWidth = static_cast<float>(client.right - client.left);
		const float clientHeight = static_cast<float>(client.bottom - client.top);
		if (clientWidth <= 1.0f || clientHeight <= 1.0f)
			return scaled;

		const ImVec2 displaySize = GetOverlayDisplaySize();
		const float displayWidth = displaySize.x;
		const float displayHeight = displaySize.y;

		scaled.x *= displayWidth / clientWidth;
		scaled.y *= displayHeight / clientHeight;
		return scaled;
	}

	bool TryReadWindowCursor(ImVec2& point)
	{
		HWND window = CMessageProxy::GetWindowHandle();
		if (!window)
			return false;

		POINT cursor = {};
		if (!GetCursorPos(&cursor) || !ScreenToClient(window, &cursor))
			return false;

		point = ScaleClientPointToDisplay(window, cursor);
		return true;
	}

	void MoveWindowCursorToDisplayPoint(const ImVec2& point)
	{
		HWND window = CMessageProxy::GetWindowHandle();
		if (!window)
			return;

		RECT client = {};
		if (!GetClientRect(window, &client))
			return;

		const float clientWidth = static_cast<float>(client.right - client.left);
		const float clientHeight = static_cast<float>(client.bottom - client.top);
		const ImVec2 displaySize = GetOverlayDisplaySize();
		const float displayWidth = displaySize.x;
		const float displayHeight = displaySize.y;
		if (clientWidth <= 1.0f || clientHeight <= 1.0f || displayWidth <= 1.0f || displayHeight <= 1.0f)
			return;

		POINT cursor = {};
		cursor.x = static_cast<LONG>(ClampFloat(point.x * clientWidth / displayWidth, 0.0f, clientWidth - 1.0f));
		cursor.y = static_cast<LONG>(ClampFloat(point.y * clientHeight / displayHeight, 0.0f, clientHeight - 1.0f));
		if (ClientToScreen(window, &cursor))
			SetCursorPos(cursor.x, cursor.y);
	}

	const char* ResultLabel(unsigned char result)
	{
		switch (result)
		{
		case OMPPlusProtocol::BuildResultSuccess:
			return "OK";
		case OMPPlusProtocol::BuildResultPreviewValid:
			return "VALID";
		case OMPPlusProtocol::BuildResultPreviewInvalid:
			return "INVALID";
		default:
			return "ERROR";
		}
	}
}

void CBuildManager::HandleOpen(RakNet::BitStream& bitStream)
{
	unsigned int sessionId = 0;
	float maxDistance = 8.0f;
	std::string title;
	if (!bitStream.Read(sessionId) || !bitStream.Read(maxDistance) || !ReadBoundString(bitStream, title, 48) || sessionId == 0)
		return;

	cScopedBuildStateLock lock;
	ClearUnlocked(true);
	CTargetManager::ClearContext();

	m_active = true;
	m_menuOpen = true;
	m_sessionId = sessionId;
	m_maxDistance = maxDistance > 0.0f ? maxDistance : 8.0f;
	m_title = title.empty() ? "Build Mode" : title;
	m_selectedPartId = 0;
	m_rotationStep = 0;
	m_flipped = false;
	m_status.clear();
	m_statusUntil = 0;
	m_inputGuardUntil = GetTickCount() + 450;
	m_lastLeftDown = m_mouseButtons[0] || IsPhysicalKeyDown(VK_LBUTTON);
	m_lastRightDown = m_mouseButtons[1] || IsPhysicalKeyDown(VK_RBUTTON);
	m_lastMiddleDown = m_mouseButtons[2] || IsPhysicalKeyDown(VK_MBUTTON);
	m_lastEscapeDown = IsPhysicalKeyDown(VK_ESCAPE);
	m_lastQDown = IsPhysicalKeyDown('Q');
	m_lastEDown = IsPhysicalKeyDown('E');
	memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
	m_virtualCursorInitialized = false;
	m_cursorOwned = !CGraphics::IsCursorEnabled();
	CGraphics::ToggleCursor(true);
	CDInput8DeviceProxy::RequestInputReset();
	CLog::Write("Build UI opened session=%u", sessionId);
}

void CBuildManager::HandleClose()
{
	Clear();
}

void CBuildManager::HandleClearParts()
{
	cScopedBuildStateLock lock;
	m_parts.clear();
	m_selectedPartId = 0;
}

void CBuildManager::HandleAddPart(RakNet::BitStream& bitStream)
{
	sBuildPart part = {};
	if (!bitStream.Read(part.partId) || !bitStream.Read(part.modelId))
		return;
	if (!ReadBoundString(bitStream, part.name, 48)
		|| !ReadBoundString(bitStream, part.category, 32)
		|| !ReadBoundString(bitStream, part.cost, 48))
		return;
	if (part.partId == 0 || part.name.empty())
		return;

	cScopedBuildStateLock lock;
	const auto existing = std::find_if(m_parts.begin(), m_parts.end(), [part](const sBuildPart& current)
	{
		return current.partId == part.partId;
	});
	if (existing != m_parts.end())
		*existing = part;
	else
		m_parts.push_back(part);

	// Do not auto-select the first part. Preview starts only after the player
	// deliberately chooses a part, then the client enters placement mode.
}

void CBuildManager::HandleResult(RakNet::BitStream& bitStream)
{
	unsigned char result = 0;
	std::string message;
	if (!bitStream.Read(result) || !ReadBoundString(bitStream, message, 96))
		return;

	cScopedBuildStateLock lock;
	m_statusSuccess = result == OMPPlusProtocol::BuildResultSuccess || result == OMPPlusProtocol::BuildResultPreviewValid;
	m_status = message.empty() ? ResultLabel(result) : message;
	m_statusUntil = GetTickCount() + 2200;
}

void CBuildManager::Clear()
{
	cScopedBuildStateLock lock;
	ClearUnlocked(true);
}

void CBuildManager::ClearUnlocked(bool restoreCursor)
{
	const bool wasActive = m_active;
	m_active = false;
	m_menuOpen = false;
	m_sessionId = 0;
	m_selectedPartId = 0;
	m_maxDistance = 8.0f;
	m_rotationStep = 0;
	m_flipped = false;
	m_inputGuardUntil = 0;
	m_status.clear();
	m_statusUntil = 0;
	m_title.clear();
	m_parts.clear();
	m_virtualCursorInitialized = false;
	m_virtualWheel = 0;
	memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
	m_lastLeftDown = m_lastRightDown = m_lastMiddleDown = false;
	m_lastEscapeDown = m_lastQDown = m_lastEDown = false;
	m_menuX = m_menuY = m_menuW = m_menuH = 0.0f;

	if (wasActive)
		CDInput8DeviceProxy::RequestInputReset();

	if (restoreCursor && m_cursorOwned)
	{
		CGraphics::ToggleCursor(false);
		m_cursorOwned = false;
	}
}

void CBuildManager::Process()
{
	cScopedBuildStateLock lock;
	if (!m_active)
		return;

	const bool escapeDown = IsPhysicalKeyDown(VK_ESCAPE);
	const bool rightDown = m_mouseButtons[1] || IsPhysicalKeyDown(VK_RBUTTON);
	const bool leftDown = m_mouseButtons[0] || IsPhysicalKeyDown(VK_LBUTTON);
	const bool middleDown = m_mouseButtons[2] || IsPhysicalKeyDown(VK_MBUTTON);
	const bool qDown = IsPhysicalKeyDown('Q');
	const bool eDown = IsPhysicalKeyDown('E');
	const bool inputGuardActive = static_cast<long>(GetTickCount() - m_inputGuardUntil) < 0;

	if (!inputGuardActive && ((escapeDown && !m_lastEscapeDown) || (rightDown && !m_lastRightDown)))
	{
		SendCancel();
		return;
	}

	if (m_menuOpen)
	{
		m_lastEscapeDown = escapeDown;
		m_lastRightDown = rightDown;
		m_lastLeftDown = leftDown;
		m_lastMiddleDown = middleDown;
		m_lastQDown = qDown;
		m_lastEDown = eDown;
		return;
	}

	if (qDown && !m_lastQDown)
	{
		m_rotationStep = (m_rotationStep + 3) % 4;
		SendPreviewState();
	}
	if (eDown && !m_lastEDown)
	{
		m_rotationStep = (m_rotationStep + 1) % 4;
		SendPreviewState();
	}
	if (middleDown && !m_lastMiddleDown)
	{
		m_flipped = !m_flipped;
		SendPreviewState();
	}

	if (leftDown && !m_lastLeftDown && m_selectedPartId != 0 && !IsMouseOverMenu())
		SendPlace();

	m_lastEscapeDown = escapeDown;
	m_lastRightDown = rightDown;
	m_lastLeftDown = leftDown;
	m_lastMiddleDown = middleDown;
	m_lastQDown = qDown;
	m_lastEDown = eDown;
}

void CBuildManager::RenderImGui()
{
	cScopedBuildStateLock lock;
	if (!m_active)
		return;

	if (!m_menuOpen)
		return;

	ApplyImGuiInput();

	const ImVec2 displaySize = GetOverlayDisplaySize();
	const float width = (std::max)(280.0f, (std::min)(340.0f, displaySize.x * 0.26f));
	const float x = displaySize.x - width - 42.0f;
	const float y = 118.0f;

	ImGui::SetNextWindowPos(ImVec2(x, y), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, 0.0f), ImGuiCond_Always);
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 9.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(7.0f, 6.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.01f, 0.012f, 0.94f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.95f, 0.95f, 0.92f, 0.45f));

	if (ImGui::Begin("##ompplus_build_menu", NULL, flags))
	{
		ImGui::TextUnformatted(m_title.empty() ? "Build Mode" : m_title.c_str());
		ImGui::Separator();

		if (m_parts.empty())
		{
			ImGui::TextWrapped("No build parts were sent by the server.");
		}
		else
		{
			std::string currentCategory;
			for (const sBuildPart& part : m_parts)
			{
				if (part.category != currentCategory)
				{
					currentCategory = part.category;
					if (!currentCategory.empty())
					{
						ImGui::Spacing();
						ImGui::TextUnformatted(currentCategory.c_str());
					}
				}

				const bool selected = part.partId == m_selectedPartId;
				ImVec4 normal = selected ? ImVec4(0.24f, 0.25f, 0.25f, 0.98f) : ImVec4(0.025f, 0.030f, 0.032f, 0.94f);
				ImVec4 hover = ImVec4(0.34f, 0.35f, 0.35f, 1.00f);
				ImVec4 active = ImVec4(0.46f, 0.47f, 0.47f, 1.00f);
				ImGui::PushStyleColor(ImGuiCol_Button, normal);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hover);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, active);

				char label[160] = {};
				if (part.cost.empty())
					std::snprintf(label, sizeof(label), "%s##buildpart%u", part.name.c_str(), part.partId);
				else
					std::snprintf(label, sizeof(label), "%s    %s##buildpart%u", part.name.c_str(), part.cost.c_str(), part.partId);

				if (ImGui::Button(label, ImVec2(-1.0f, 30.0f)))
				{
					m_selectedPartId = part.partId;
					m_rotationStep = 0;
					m_flipped = false;
					m_menuOpen = false;
					if (m_cursorOwned)
					{
						CGraphics::ToggleCursor(false);
						m_cursorOwned = false;
					}
					m_lastLeftDown = m_mouseButtons[0] || IsPhysicalKeyDown(VK_LBUTTON);
					m_lastRightDown = m_mouseButtons[1] || IsPhysicalKeyDown(VK_RBUTTON);
					m_lastMiddleDown = m_mouseButtons[2] || IsPhysicalKeyDown(VK_MBUTTON);
					m_lastQDown = IsPhysicalKeyDown('Q');
					m_lastEDown = IsPhysicalKeyDown('E');
					m_virtualCursorInitialized = false;
					CDInput8DeviceProxy::RequestInputReset();
					SendSelect(part.partId);
					SendPreviewState();
				}

				ImGui::PopStyleColor(3);
			}
		}

		ImGui::Spacing();
		ImGui::Separator();
		char footer[96] = {};
		std::snprintf(footer, sizeof(footer), "Rot %d deg%s  |  LMB place  RMB close", (m_rotationStep % 4) * 90, m_flipped ? " flipped" : "");
		ImGui::TextWrapped("%s", footer);

		if (!m_status.empty() && GetTickCount() <= m_statusUntil)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, m_statusSuccess ? ImVec4(0.82f, 1.0f, 0.82f, 1.0f) : ImVec4(1.0f, 0.72f, 0.72f, 1.0f));
			ImGui::TextWrapped("%s", m_status.c_str());
			ImGui::PopStyleColor();
		}

		ImVec2 pos = ImGui::GetWindowPos();
		ImVec2 size = ImGui::GetWindowSize();
		m_menuX = pos.x;
		m_menuY = pos.y;
		m_menuW = size.x;
		m_menuH = size.y;
	}
	ImGui::End();

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(2);
}

bool CBuildManager::IsActive()
{
	return m_active;
}

bool CBuildManager::IsMenuOpen()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldCaptureMouse()
{
	return m_active;
}

bool CBuildManager::ShouldPassMouseMovement()
{
	return m_active && !m_menuOpen && m_selectedPartId != 0;
}

bool CBuildManager::ShouldBlockCursorMove()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldSuppressKeyboard()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldNeutralizeKeyboard()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldBlockGameControls()
{
	return m_active && m_menuOpen;
}

bool CBuildManager::ShouldConsumeDirectInputEvent(DWORD offset, DWORD)
{
	if (!m_active)
		return false;
	if (m_menuOpen)
		return true;
	return offset == DIK_Q || offset == DIK_E || offset == DIK_ESCAPE;
}

void CBuildManager::FilterKeyboardState(DWORD size, LPVOID state)
{
	if (!m_active || !state)
		return;

	if (m_menuOpen)
	{
		std::memset(state, 0, size);
		return;
	}

	if (size >= 256)
	{
		BYTE* keyboard = reinterpret_cast<BYTE*>(state);
		keyboard[DIK_Q] = 0;
		keyboard[DIK_E] = 0;
		keyboard[DIK_ESCAPE] = 0;
	}
}

void CBuildManager::FilterMouseState(DWORD size, LPVOID state)
{
	if (!m_active || !state)
		return;

	if (size >= sizeof(DIMOUSESTATE2))
	{
		DIMOUSESTATE2* mouse = reinterpret_cast<DIMOUSESTATE2*>(state);
		if (m_menuOpen)
		{
			mouse->lX = 0;
			mouse->lY = 0;
		}
		std::memset(mouse->rgbButtons, 0, sizeof(mouse->rgbButtons));
		mouse->lZ = 0;
	}
	else if (size >= sizeof(DIMOUSESTATE))
	{
		DIMOUSESTATE* mouse = reinterpret_cast<DIMOUSESTATE*>(state);
		if (m_menuOpen)
		{
			mouse->lX = 0;
			mouse->lY = 0;
		}
		std::memset(mouse->rgbButtons, 0, sizeof(mouse->rgbButtons));
		mouse->lZ = 0;
	}
}

DWORD CBuildManager::GetKeyboardReleaseOffsets(DWORD* offsets, DWORD capacity)
{
	if (!m_active || !offsets || !capacity)
		return 0;

	if (m_menuOpen)
	{
		const DWORD available = static_cast<DWORD>(sizeof(KeyboardReleaseKeys) / sizeof(KeyboardReleaseKeys[0]));
		const DWORD count = capacity < available ? capacity : available;
		for (DWORD i = 0; i < count; ++i)
			offsets[i] = KeyboardReleaseKeys[i].offset;
		return count;
	}

	const DWORD placementOffsets[] = { DIK_Q, DIK_E, DIK_ESCAPE };
	const DWORD available = static_cast<DWORD>(sizeof(placementOffsets) / sizeof(placementOffsets[0]));
	const DWORD count = capacity < available ? capacity : available;
	for (DWORD i = 0; i < count; ++i)
		offsets[i] = placementOffsets[i];
	return count;
}

void CBuildManager::AddMouseDelta(LONG x, LONG y, LONG wheel)
{
	if (!m_active)
		return;

	if (!m_virtualCursorInitialized)
		InitializeVirtualCursor();

	m_virtualCursorX += static_cast<float>(x);
	m_virtualCursorY += static_cast<float>(y);
	m_virtualWheel += wheel;
}

void CBuildManager::SetWindowMousePosition(LONG x, LONG y)
{
	if (!m_active)
		return;

	POINT point = { x, y };
	const ImVec2 scaled = ScaleClientPointToDisplay(CMessageProxy::GetWindowHandle(), point);
	m_virtualCursorX = scaled.x;
	m_virtualCursorY = scaled.y;
	m_virtualCursorInitialized = true;
}

void CBuildManager::SetMouseButton(unsigned int button, bool down)
{
	if (button >= 5)
		return;
	m_mouseButtons[button] = down;
}

bool CBuildManager::ReadBoundString(RakNet::BitStream& bitStream, std::string& value, unsigned char maxLength)
{
	unsigned char length = 0;
	if (!bitStream.Read(length) || length > maxLength)
		return false;

	char buffer[128] = {};
	if (length && !bitStream.Read(buffer, length))
		return false;

	value.assign(buffer, length);
	return true;
}

void CBuildManager::InitializeVirtualCursor()
{
	const ImVec2 displaySize = GetOverlayDisplaySize();
	const float width = displaySize.x;

	const float menuWidth = (std::max)(280.0f, (std::min)(340.0f, width * 0.26f));
	m_virtualCursorX = width - menuWidth * 0.5f - 42.0f;
	m_virtualCursorY = 210.0f;
	m_virtualCursorInitialized = true;
	MoveWindowCursorToDisplayPoint(ImVec2(m_virtualCursorX, m_virtualCursorY));
}

void CBuildManager::ApplyImGuiInput()
{
	ImGuiIO& io = ImGui::GetIO();

	if (!m_virtualCursorInitialized)
		InitializeVirtualCursor();

	ImVec2 windowCursor;
	if (TryReadWindowCursor(windowCursor))
	{
		m_virtualCursorX = windowCursor.x;
		m_virtualCursorY = windowCursor.y;
	}

	const float maxX = (std::max)(1.0f, io.DisplaySize.x - 1.0f);
	const float maxY = (std::max)(1.0f, io.DisplaySize.y - 1.0f);
	m_virtualCursorX = ClampFloat(m_virtualCursorX, 0.0f, maxX);
	m_virtualCursorY = ClampFloat(m_virtualCursorY, 0.0f, maxY);

	io.MousePos = ImVec2(m_virtualCursorX, m_virtualCursorY);
	io.MouseDown[0] = m_mouseButtons[0] || IsPhysicalKeyDown(VK_LBUTTON);
	io.MouseDown[1] = m_mouseButtons[1] || IsPhysicalKeyDown(VK_RBUTTON);
	io.MouseDown[2] = m_mouseButtons[2] || IsPhysicalKeyDown(VK_MBUTTON);
	for (int i = 3; i < 5; ++i)
		io.MouseDown[i] = m_mouseButtons[i];

	if (m_virtualWheel != 0)
	{
		io.MouseWheel += static_cast<float>(m_virtualWheel) / 120.0f;
		m_virtualWheel = 0;
	}
}

bool CBuildManager::IsMouseOverMenu()
{
	if (m_menuW <= 0.0f || m_menuH <= 0.0f)
		return true;
	return m_virtualCursorX >= m_menuX && m_virtualCursorX <= m_menuX + m_menuW
		&& m_virtualCursorY >= m_menuY && m_virtualCursorY <= m_menuY + m_menuH;
}

void CBuildManager::SendSelect(unsigned int partId)
{
	if (!Network::IsConnected() || !m_active || m_sessionId == 0 || partId == 0)
		return;

	RakNet::BitStream bitStream;
	bitStream.Write(m_sessionId);
	bitStream.Write(partId);
	Network::SendRPC(eRPC::ON_BUILD_SELECT, &bitStream);
}

void CBuildManager::SendPreviewState()
{
	if (!Network::IsConnected() || !m_active || m_sessionId == 0 || m_selectedPartId == 0)
		return;

	RakNet::BitStream bitStream;
	bitStream.Write(m_sessionId);
	bitStream.Write(m_selectedPartId);
	bitStream.Write(static_cast<short>(m_rotationStep));
	bitStream.Write(m_flipped);
	Network::SendRPC(eRPC::ON_BUILD_PREVIEW, &bitStream);
}

void CBuildManager::SendPlace()
{
	if (!Network::IsConnected() || !m_active || m_sessionId == 0 || m_selectedPartId == 0)
		return;

	RakNet::BitStream bitStream;
	bitStream.Write(m_sessionId);
	bitStream.Write(m_selectedPartId);
	bitStream.Write(static_cast<short>(m_rotationStep));
	bitStream.Write(m_flipped);
	Network::SendRPC(eRPC::ON_BUILD_PLACE, &bitStream);
}

void CBuildManager::SendCancel()
{
	if (Network::IsConnected() && m_active && m_sessionId != 0)
	{
		RakNet::BitStream bitStream;
		bitStream.Write(m_sessionId);
		Network::SendRPC(eRPC::ON_BUILD_CANCEL, &bitStream);
	}

	ClearUnlocked(true);
}
