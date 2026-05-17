#include <SAMP+/CRPC.h>
#include <SAMP+/OMPPlusProtocol.h>
#include <SAMP+/client/CGraphics.h>
#include <SAMP+/client/CKeyBinds.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/COverlayLayout.h>
#include <SAMP+/client/COverlayRenderer.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/Proxy/CDInput8DeviceProxy.h>
#include <SAMP+/client/Proxy/CMessageProxy.h>

#include <DirectX/dinput.h>
#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <cstring>

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
unsigned char CTargetManager::m_targetType = OMPPlusProtocol::TargetTypeGeneric;
unsigned char CTargetManager::m_layout = OMPPlusProtocol::TargetLayoutAuto;
unsigned long CTargetManager::m_expiresAt = 0;
unsigned long CTargetManager::m_lastContextTick = 0;
unsigned long CTargetManager::m_mouseSuppressUntil = 0;
unsigned long CTargetManager::m_inputGuardUntil = 0;
unsigned long CTargetManager::m_keyboardReleaseUntil = 0;
bool CTargetManager::m_keyboardReleaseActive = false;
bool CTargetManager::m_keyboardReleaseOffsets[256] = {};
std::string CTargetManager::m_title;
std::string CTargetManager::m_description;
std::vector<sTargetOption> CTargetManager::m_options;
int CTargetManager::m_hoverIndex = -1;
POINT CTargetManager::m_cursor = { 0, 0 };
ID3DXFont* CTargetManager::m_font = NULL;

namespace
{
	struct sTargetStateLock
	{
		CRITICAL_SECTION section;

		sTargetStateLock()
		{
			InitializeCriticalSection(&section);
		}

		~sTargetStateLock()
		{
			DeleteCriticalSection(&section);
		}
	};

	class cScopedTargetStateLock
	{
	public:
		cScopedTargetStateLock()
		{
			EnterCriticalSection(&TargetLock().section);
		}

		~cScopedTargetStateLock()
		{
			LeaveCriticalSection(&TargetLock().section);
		}

	private:
		static sTargetStateLock& TargetLock()
		{
			static sTargetStateLock lock;
			return lock;
		}
	};

	const unsigned char MaxTargetOptions = 8;
	const unsigned char MaxTargetRows = 12;
	const float PromptLegendOffsetX = 45.0f;
	const float PromptTitleHeight = 24.0f;
	const float PromptKeyWidth = 36.0f;
	const float PromptKeyHeight = 22.0f;
	const float PromptLegendGap = 4.0f;
	const float PromptTitleMinWidth = 82.0f;
	const float PromptTitleMaxWidth = 220.0f;
	const float HeaderHeight = 30.0f;
	const float RowHeight = 30.0f;
	const float DialogRowHeight = 64.0f;
	const float InfoRowHeight = 34.0f;
	const float DividerRowHeight = 10.0f;
	const float RowGap = 4.0f;
	const unsigned long PostMouseSuppressMs = 180;
	const float VirtualMouseSensitivity = 1.0f;

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
		{ DIK_RBRACKET, VK_OEM_6 }
	};

	bool IsPhysicalKeyDown(int virtualKey)
	{
		return (GetAsyncKeyState(virtualKey) & 0x8000) != 0;
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

		CPoint2D& resolution = CGraphics::GetScreenResolution();
		const float displayWidth = static_cast<float>(resolution.X());
		const float displayHeight = static_cast<float>(resolution.Y());
		if (displayWidth <= 1.0f || displayHeight <= 1.0f)
			return scaled;

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
		CPoint2D& resolution = CGraphics::GetScreenResolution();
		const float displayWidth = static_cast<float>(resolution.X());
		const float displayHeight = static_cast<float>(resolution.Y());
		if (clientWidth <= 1.0f || clientHeight <= 1.0f || displayWidth <= 1.0f || displayHeight <= 1.0f)
			return;

		POINT cursor = {};
		cursor.x = static_cast<LONG>((std::max)(0.0f, (std::min)(point.x * clientWidth / displayWidth, clientWidth - 1.0f)));
		cursor.y = static_cast<LONG>((std::max)(0.0f, (std::min)(point.y * clientHeight / displayHeight, clientHeight - 1.0f)));
		if (ClientToScreen(window, &cursor))
			SetCursorPos(cursor.x, cursor.y);
	}

	float ClampFloat(float value, float minValue, float maxValue)
	{
		return (std::max)(minValue, (std::min)(value, maxValue));
	}

	unsigned char NormalizeTargetType(unsigned char targetType)
	{
		return targetType <= OMPPlusProtocol::TargetTypeCustom ? targetType : OMPPlusProtocol::TargetTypeGeneric;
	}

	unsigned char NormalizeTargetLayout(unsigned char layout)
	{
		return layout <= OMPPlusProtocol::TargetLayoutCategory ? layout : OMPPlusProtocol::TargetLayoutAuto;
	}

	unsigned char NormalizeTargetRowType(unsigned char rowType)
	{
		return rowType <= OMPPlusProtocol::TargetRowDanger ? rowType : OMPPlusProtocol::TargetRowAction;
	}
}

void CTargetManager::HandleSetContext(RakNet::BitStream& bitStream)
{
	unsigned int targetId = 0;
	unsigned short ttlMs = 0;
	unsigned int flags = 0;
	unsigned char targetType = OMPPlusProtocol::TargetTypeGeneric;
	unsigned char layout = OMPPlusProtocol::TargetLayoutAuto;
	std::string title;
	std::string description;
	unsigned char optionCount = 0;

	if (!bitStream.Read(targetId)
		|| !bitStream.Read(ttlMs)
		|| !bitStream.Read(flags)
		|| targetId == 0)
	{
		return;
	}

	const bool payloadV2 = (flags & OMPPlusProtocol::TargetFlagPayloadV2) != 0;
	if (payloadV2)
	{
		if (!bitStream.Read(targetType)
			|| !bitStream.Read(layout)
			|| !ReadBoundString(bitStream, title, 48)
			|| !ReadBoundString(bitStream, description, 128)
			|| !bitStream.Read(optionCount)
			|| optionCount > MaxTargetRows)
		{
			return;
		}
		targetType = NormalizeTargetType(targetType);
		layout = NormalizeTargetLayout(layout);
	}
	else
	{
		if (!ReadBoundString(bitStream, title, 48)
			|| !bitStream.Read(optionCount)
			|| optionCount > MaxTargetOptions)
		{
			return;
		}
	}

	std::vector<sTargetOption> options;
	for (unsigned char i = 0; i < optionCount; ++i)
	{
		sTargetOption option;
		option.rowType = OMPPlusProtocol::TargetRowAction;

		if (!bitStream.Read(option.optionId))
		{
			return;
		}

		if (payloadV2)
		{
			if (!bitStream.Read(option.rowType)
				|| !bitStream.Read(option.enabled)
				|| !ReadBoundString(bitStream, option.label, 96)
				|| !ReadBoundString(bitStream, option.icon, 24))
			{
				return;
			}
			option.rowType = NormalizeTargetRowType(option.rowType);
		}
		else
		{
			if (!bitStream.Read(option.enabled)
				|| !ReadBoundString(bitStream, option.label, 48)
				|| !ReadBoundString(bitStream, option.icon, 24)
				|| option.optionId == 0)
			{
				return;
			}
			if (!option.enabled)
				option.rowType = OMPPlusProtocol::TargetRowDisabled;
		}

		if (IsSelectableRow(option.rowType) && option.optionId == 0)
			return;
		options.push_back(option);
	}

	cScopedTargetStateLock lock;

	if (ttlMs == 0)
		ttlMs = 500;
	if (ttlMs > 5000)
		ttlMs = 5000;

	m_hasContext = true;
	m_targetId = targetId;
	m_flags = flags & ~OMPPlusProtocol::TargetFlagPayloadV2;
	m_targetType = targetType;
	m_layout = layout;
	m_title = title;
	m_description = description;
	m_options = options;
	m_expiresAt = GetTickCount() + ttlMs;
	m_lastContextTick = GetTickCount();

	if (m_options.empty())
		ClearContextUnlocked();
}

void CTargetManager::ClearContext()
{
	cScopedTargetStateLock lock;

	ClearContextUnlocked();
}

void CTargetManager::ClearContextUnlocked()
{
	CloseMenu(true);
	m_hasContext = false;
	m_targetId = 0;
	m_flags = 0;
	m_targetType = OMPPlusProtocol::TargetTypeGeneric;
	m_layout = OMPPlusProtocol::TargetLayoutAuto;
	m_expiresAt = 0;
	m_title.clear();
	m_description.clear();
	m_options.clear();
	m_hoverIndex = -1;
	m_lastContextTick = 0;
}

void CTargetManager::Process()
{
	cScopedTargetStateLock lock;

	UpdateKeyboardReleaseLease();

	const bool altDown = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
	const bool escapeDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
	const bool rightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	const bool inputGuardActive = static_cast<long>(GetTickCount() - m_inputGuardUntil) < 0;

	if (!HasValidContext())
	{
		if (m_hasContext)
			ClearContextUnlocked();
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
		if (IsDirectSelectContext())
		{
			SendDirectSelect();
		}
		else if (m_menuOpen)
		{
			if (!inputGuardActive)
			{
				CloseMenu(true);
				m_expiresAt = 0;
			}
		}
		else
			OpenMenu();
	}
	else if (m_menuOpen && !inputGuardActive && ((escapeDown && !m_lastEscapeDown) || (rightDown && !m_lastRightDown)))
	{
		CloseMenu(true);
		m_expiresAt = 0;
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
		if (option.enabled && IsSelectableRow(option.rowType))
			SendSelect(option.optionId);
	}
	m_lastLeftDown = leftDown;
}

void CTargetManager::Draw(IDirect3DDevice9* device)
{
	cScopedTargetStateLock lock;

	if (!device || !HasValidContext())
		return;

	EnsureFont(device);

	CPoint2D& resolution = CGraphics::GetScreenResolution();
	const float cx = OverlayLayout::GetTargetCenterX(static_cast<float>(resolution.X()));
	const float cy = static_cast<float>(resolution.Y()) * 0.5f;

	if (m_menuOpen)
		DrawMenu(device, cx, cy);
	else if (ShouldDrawPrompt())
	{
		DrawEye(device, cx, cy);
		DrawPromptLegend(device, cx, cy);
	}
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
	cScopedTargetStateLock lock;

	CloseMenu(true);
	if (m_font)
	{
		m_font->Release();
		m_font = NULL;
	}
}

bool CTargetManager::IsMenuOpen()
{
	cScopedTargetStateLock lock;

	return m_menuOpen;
}

bool CTargetManager::HasActiveContext()
{
	cScopedTargetStateLock lock;

	return m_hasContext && m_targetId != 0 && !m_options.empty() && static_cast<long>(GetTickCount() - m_expiresAt) < 0;
}

bool CTargetManager::ShouldCaptureMouse()
{
	cScopedTargetStateLock lock;

	return (m_menuOpen || IsMouseSuppressed()) && InputAllowed();
}

bool CTargetManager::ShouldSuppressKeyboard()
{
	cScopedTargetStateLock lock;

	return HasKeyboardReleaseLease() && InputAllowed();
}

bool CTargetManager::ShouldNeutralizeKeyboard()
{
	cScopedTargetStateLock lock;

	return InputAllowed() && (m_menuOpen || HasKeyboardReleaseLease());
}

bool CTargetManager::ShouldBlockGameControls()
{
	cScopedTargetStateLock lock;

	if (!InputAllowed())
		return false;

	return m_menuOpen || HasKeyboardReleaseLease() || IsMouseSuppressed();
}

bool CTargetManager::ShouldConsumeDirectInputOffset(DWORD offset)
{
	cScopedTargetStateLock lock;

	if (!InputAllowed())
		return false;

	UpdateKeyboardReleaseLease();
	if (offset < 256 && m_keyboardReleaseActive && m_keyboardReleaseOffsets[offset])
		return true;

	if (m_hasContext && m_targetId != 0 && !m_options.empty() && static_cast<long>(GetTickCount() - m_expiresAt) < 0)
		return offset == DIK_LMENU || offset == DIK_RMENU;

	return false;
}

bool CTargetManager::ShouldConsumeDirectInputEvent(DWORD offset, DWORD data)
{
	cScopedTargetStateLock lock;

	if (!InputAllowed())
		return false;

	const bool keyDown = (data & 0x80) != 0;
	UpdateKeyboardReleaseLease();
	if (offset < 256 && m_keyboardReleaseActive && m_keyboardReleaseOffsets[offset])
		return true;

	if (m_hasContext && m_targetId != 0 && !m_options.empty() && static_cast<long>(GetTickCount() - m_expiresAt) < 0)
		return keyDown && (offset == DIK_LMENU || offset == DIK_RMENU);

	return false;
}

void CTargetManager::FilterKeyboardState(DWORD size, LPVOID state)
{
	cScopedTargetStateLock lock;

	if (!state || !size)
		return;

	if (!InputAllowed())
		return;

	BYTE* keys = static_cast<BYTE*>(state);
	UpdateKeyboardReleaseLease();

	if (m_menuOpen)
	{
		memset(state, 0, size);
		return;
	}

	if (m_keyboardReleaseActive)
	{
		for (unsigned int i = 0; i < sizeof(KeyboardReleaseKeys) / sizeof(KeyboardReleaseKeys[0]); ++i)
		{
			const DWORD offset = KeyboardReleaseKeys[i].offset;
			if (offset < size && m_keyboardReleaseOffsets[offset])
				keys[offset] = 0;
		}
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
	cScopedTargetStateLock lock;

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

bool CTargetManager::ShouldReleaseDirectInputOffset(DWORD offset)
{
	cScopedTargetStateLock lock;

	UpdateKeyboardReleaseLease();
	return offset < 256 && m_keyboardReleaseActive && m_keyboardReleaseOffsets[offset];
}

DWORD CTargetManager::GetKeyboardReleaseOffsets(DWORD* offsets, DWORD capacity)
{
	cScopedTargetStateLock lock;

	if (!offsets || !capacity)
		return 0;

	UpdateKeyboardReleaseLease();
	if (!m_keyboardReleaseActive)
		return 0;

	DWORD count = 0;
	for (unsigned int i = 0; i < sizeof(KeyboardReleaseKeys) / sizeof(KeyboardReleaseKeys[0]) && count < capacity; ++i)
	{
		const DWORD offset = KeyboardReleaseKeys[i].offset;
		if (offset < 256 && m_keyboardReleaseOffsets[offset])
			offsets[count++] = offset;
	}
	return count;
}

void CTargetManager::AddMouseDelta(LONG x, LONG y, LONG wheel)
{
	cScopedTargetStateLock lock;

	if (!m_menuOpen)
		return;

	if (!m_virtualCursorInitialized)
		InitializeVirtualCursor();

	m_virtualCursorX += static_cast<float>(x) * VirtualMouseSensitivity;
	m_virtualCursorY += static_cast<float>(y) * VirtualMouseSensitivity;
	m_virtualWheel += wheel;
}

void CTargetManager::SetWindowMousePosition(LONG x, LONG y)
{
	cScopedTargetStateLock lock;

	if (!m_menuOpen)
		return;

	POINT point = { x, y };
	const ImVec2 scaled = ScaleClientPointToDisplay(CMessageProxy::GetWindowHandle(), point);
	m_virtualCursorX = scaled.x;
	m_virtualCursorY = scaled.y;
	m_virtualCursorInitialized = true;
}

void CTargetManager::SetMouseButton(unsigned int button, bool down)
{
	cScopedTargetStateLock lock;

	if (button >= 5)
		return;
	if (!m_menuOpen)
		return;

	m_mouseButtons[button] = down;
}

void CTargetManager::RenderImGui()
{
	cScopedTargetStateLock lock;

	if (!HasValidContext() || !InputAllowed())
		return;

	ImGuiIO& io = ImGui::GetIO();
	ApplyImGuiInput();

	const float cx = OverlayLayout::GetTargetCenterX(io.DisplaySize.x);
	const float cy = io.DisplaySize.y * 0.5f;
	const ImU32 black = IM_COL32(5, 7, 8, 220);
	const ImU32 blackSoft = IM_COL32(0, 0, 0, 128);
	const ImU32 blackLine = IM_COL32(0, 0, 0, 230);
	const ImU32 accent = IM_COL32(245, 248, 246, 235);
	const ImU32 accentSoft = IM_COL32(245, 248, 246, 92);
	const ImU32 textSoft = IM_COL32(232, 234, 232, 220);

	ImDrawList* draw = ImGui::GetForegroundDrawList();
	const bool drawPrompt = ShouldDrawPrompt();
	if (drawPrompt)
	{
		draw->AddCircleFilled(ImVec2(cx, cy), 19.0f, blackSoft, 40);
		draw->AddCircle(ImVec2(cx, cy), 18.0f, blackLine, 40, 2.5f);
		draw->AddCircle(ImVec2(cx, cy), 12.0f, accentSoft, 40, 1.5f);
		draw->AddCircleFilled(ImVec2(cx, cy), 4.5f, accent, 32);
		draw->AddLine(ImVec2(cx - 39.0f, cy), ImVec2(cx - 24.0f, cy), blackLine, 3.0f);
		draw->AddLine(ImVec2(cx + 24.0f, cy), ImVec2(cx + 39.0f, cy), blackLine, 3.0f);
		draw->AddLine(ImVec2(cx - 36.0f, cy), ImVec2(cx - 24.0f, cy), accentSoft, 1.4f);
		draw->AddLine(ImVec2(cx + 24.0f, cy), ImVec2(cx + 36.0f, cy), accentSoft, 1.4f);
	}

	if (!m_menuOpen)
	{
		if (drawPrompt)
		{
			const char* title = m_title.empty() ? "Target" : m_title.c_str();
			const ImVec2 titleSize = ImGui::CalcTextSize(title);
			const float titleWidth = (std::min)(PromptTitleMaxWidth, (std::max)(PromptTitleMinWidth, titleSize.x + 18.0f));
			const float stackHeight = PromptTitleHeight + PromptLegendGap + PromptKeyHeight;
			const float stackTop = cy - stackHeight * 0.5f;
			const ImVec2 titleMin(cx + PromptLegendOffsetX, stackTop);
			const ImVec2 titleMax(titleMin.x + titleWidth, titleMin.y + PromptTitleHeight);
			const ImVec4 titleClip(titleMin.x + 8.0f, titleMin.y, titleMax.x - 8.0f, titleMax.y);
			draw->AddRectFilled(titleMin, titleMax, black, 5.0f);
			draw->AddRect(titleMin, titleMax, accentSoft, 5.0f);
			draw->AddText(ImGui::GetFont(), ImGui::GetFontSize(), ImVec2(titleMin.x + 9.0f, titleMin.y + 5.0f), textSoft, title, title + std::strlen(title), 0.0f, &titleClip);

			const ImVec2 keyMin(cx + PromptLegendOffsetX, stackTop + PromptTitleHeight + PromptLegendGap);
			const ImVec2 keyMax(keyMin.x + PromptKeyWidth, keyMin.y + PromptKeyHeight);
			draw->AddRectFilled(keyMin, keyMax, black, 5.0f);
			draw->AddRect(keyMin, keyMax, accentSoft, 5.0f);
			draw->AddText(ImVec2(keyMin.x + 9.0f, keyMin.y + 3.0f), textSoft, "ALT");
		}
		return;
	}

	const float width = GetMenuWidth();
	const float totalHeight = GetMenuHeight(width);
	ImGui::SetNextWindowPos(ImVec2(OverlayLayout::GetMenuX(io.DisplaySize.x), cy - totalHeight * 0.5f), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(width, totalHeight), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.96f);

	ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
		| ImGuiWindowFlags_NoResize
		| ImGuiWindowFlags_NoMove
		| ImGuiWindowFlags_NoSavedSettings
		| ImGuiWindowFlags_NoCollapse
		| ImGuiWindowFlags_NoScrollbar;

	unsigned int selectedOption = 0;

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.01f, 0.012f, 0.014f, 0.96f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.92f, 0.94f, 0.92f, 0.42f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.025f, 0.035f, 0.040f, 0.94f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.21f, 0.21f, 0.98f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.30f, 0.31f, 0.31f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.025f, 0.035f, 0.040f, 0.94f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.20f, 0.21f, 0.21f, 0.98f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.30f, 0.31f, 0.31f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.92f, 0.94f, 0.92f, 0.24f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.91f, 1.00f, 0.98f, 1.00f));
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 7.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
	ImGui::Begin("##ompp_target_menu", NULL, flags);
	ImGui::TextUnformatted(m_title.empty() ? "Target" : m_title.c_str());
	ImGui::Separator();

	m_hoverIndex = -1;

	if (!m_description.empty())
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.82f, 0.84f, 0.82f, 0.96f));
		ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width - 22.0f);
		ImGui::TextWrapped("%s", m_description.c_str());
		ImGui::PopTextWrapPos();
		ImGui::PopStyleColor();
		ImGui::Dummy(ImVec2(0.0f, 2.0f));
	}

	for (size_t i = 0; i < m_options.size(); ++i)
	{
		const sTargetOption& option = m_options[i];

		if (option.rowType == OMPPlusProtocol::TargetRowDivider)
		{
			ImGui::Dummy(ImVec2(0.0f, 2.0f));
			ImGui::Separator();
			ImGui::Dummy(ImVec2(0.0f, 2.0f));
			continue;
		}

		std::string label = option.label;
		if (!option.icon.empty())
			label = option.icon + "  " + label;
		const std::string visibleLabel = label;

		if (option.rowType == OMPPlusProtocol::TargetRowInfo || option.rowType == OMPPlusProtocol::TargetRowDialog || option.rowType == OMPPlusProtocol::TargetRowHeader)
		{
			const ImVec4 rowText = option.rowType == OMPPlusProtocol::TargetRowHeader
				? ImVec4(0.96f, 0.97f, 0.96f, 1.00f)
				: ImVec4(0.78f, 0.80f, 0.78f, 0.96f);
			ImGui::PushStyleColor(ImGuiCol_Text, rowText);
			ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width - 22.0f);
			ImGui::TextWrapped("%s", label.c_str());
			ImGui::PopTextWrapPos();
			ImGui::PopStyleColor();
			ImGui::Dummy(ImVec2(0.0f, option.rowType == OMPPlusProtocol::TargetRowDialog ? 4.0f : 1.0f));
			continue;
		}

		const bool selectable = option.enabled && IsSelectableRow(option.rowType);
		const float rowHeight = GetRowHeight(option);
		const float rowWidth = (std::max)(1.0f, ImGui::GetContentRegionAvail().x);
		const ImVec2 itemSize(rowWidth, rowHeight);
		const bool drawAsButton = selectable || option.rowType == OMPPlusProtocol::TargetRowDisabled;

		if (!drawAsButton)
		{
			ImGui::Dummy(itemSize);
			continue;
		}

		const ImVec4 buttonColour = option.rowType == OMPPlusProtocol::TargetRowDanger
			? ImVec4(0.14f, 0.04f, 0.04f, 0.94f)
			: ImVec4(0.025f, 0.035f, 0.040f, 0.94f);
		const ImVec4 hoverColour = option.rowType == OMPPlusProtocol::TargetRowDanger
			? ImVec4(0.50f, 0.16f, 0.16f, 1.00f)
			: ImVec4(0.42f, 0.43f, 0.43f, 1.00f);
		const ImVec4 activeColour = option.rowType == OMPPlusProtocol::TargetRowDanger
			? ImVec4(0.62f, 0.20f, 0.20f, 1.00f)
			: ImVec4(0.56f, 0.57f, 0.57f, 1.00f);
		const ImVec4 textColour = selectable
			? ImVec4(0.91f, 1.00f, 0.98f, 1.00f)
			: ImVec4(0.70f, 0.74f, 0.72f, 0.86f);

		ImGui::PushID(static_cast<int>(i));
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, UseLeftAlignedRows() ? ImVec2(0.0f, 0.5f) : ImVec2(0.5f, 0.5f));
		ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
		ImGui::PushStyleColor(ImGuiCol_Button, buttonColour);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColour);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColour);
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.92f, 0.94f, 0.92f, 0.22f));
		ImGui::PushStyleColor(ImGuiCol_Text, textColour);

		if (!selectable)
			ImGui::BeginDisabled();
		const bool pressed = ImGui::Button(visibleLabel.c_str(), itemSize);
		if (!selectable)
			ImGui::EndDisabled();

		const ImVec2 itemMin = ImGui::GetItemRectMin();
		const ImVec2 itemMax = ImGui::GetItemRectMax();
		const ImVec2 mouse = io.MousePos;
		const bool rectHovered = selectable
			&& mouse.x >= itemMin.x && mouse.x <= itemMax.x
			&& mouse.y >= itemMin.y && mouse.y <= itemMax.y;
		const bool rowHovered = selectable
			&& (rectHovered
				|| ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)
				|| ImGui::IsItemActive());
		if (rowHovered)
		{
			m_hoverIndex = static_cast<int>(i);
			ImDrawList* rowDraw = ImGui::GetWindowDrawList();
			const ImU32 overlay = option.rowType == OMPPlusProtocol::TargetRowDanger
				? IM_COL32(128, 42, 42, 240)
				: IM_COL32(108, 110, 110, 245);
			const ImU32 overlayBorder = option.rowType == OMPPlusProtocol::TargetRowDanger
				? IM_COL32(255, 228, 228, 210)
				: IM_COL32(248, 250, 248, 220);
			rowDraw->AddRectFilled(itemMin, itemMax, overlay, 5.0f);
			rowDraw->AddRect(itemMin, itemMax, overlayBorder, 5.0f);

			const ImVec2 textSize = ImGui::CalcTextSize(visibleLabel.c_str());
			const float textX = UseLeftAlignedRows()
				? itemMin.x + 12.0f
				: itemMin.x + (std::max)(0.0f, (itemMax.x - itemMin.x - textSize.x) * 0.5f);
			const float textY = itemMin.y + (std::max)(0.0f, (rowHeight - textSize.y) * 0.5f);
			rowDraw->AddText(ImVec2(textX, textY), IM_COL32(255, 255, 255, 255), visibleLabel.c_str());
		}

		ImGui::PopStyleColor(5);
		ImGui::PopStyleVar(2);
		ImGui::PopID();

		if (selectable && pressed)
		{
			selectedOption = option.optionId;
			break;
		}
	}

	ImGui::End();
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(10);

	if (selectedOption != 0)
		SendSelect(selectedOption);
}

bool CTargetManager::HasValidContext()
{
	if (!m_hasContext || m_targetId == 0 || m_options.empty())
		return false;

	const unsigned long now = GetTickCount();
	return static_cast<long>(now - m_expiresAt) < 0;
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

bool CTargetManager::HasKeyboardReleaseLease()
{
	UpdateKeyboardReleaseLease();
	return m_keyboardReleaseActive;
}

void CTargetManager::BeginKeyboardReleaseLease()
{
	ClearKeyboardReleaseLease();

	for (unsigned int i = 0; i < sizeof(KeyboardReleaseKeys) / sizeof(KeyboardReleaseKeys[0]); ++i)
	{
		const sKeyboardReleaseKey& key = KeyboardReleaseKeys[i];
		if (key.offset < 256 && IsPhysicalKeyDown(key.virtualKey))
		{
			m_keyboardReleaseOffsets[key.offset] = true;
			m_keyboardReleaseActive = true;
		}
	}

	if (m_keyboardReleaseActive)
		m_keyboardReleaseUntil = 0;
}

void CTargetManager::UpdateKeyboardReleaseLease()
{
	if (!m_keyboardReleaseActive)
		return;

	bool anyActive = false;
	for (unsigned int i = 0; i < sizeof(KeyboardReleaseKeys) / sizeof(KeyboardReleaseKeys[0]); ++i)
	{
		const sKeyboardReleaseKey& key = KeyboardReleaseKeys[i];
		if (key.offset >= 256 || !m_keyboardReleaseOffsets[key.offset])
			continue;

		if (IsPhysicalKeyDown(key.virtualKey))
		{
			anyActive = true;
			continue;
		}

		m_keyboardReleaseOffsets[key.offset] = false;
	}

	if (!anyActive)
		ClearKeyboardReleaseLease();
}

void CTargetManager::ClearKeyboardReleaseLease()
{
	const bool hadActiveLease = m_keyboardReleaseActive;
	m_keyboardReleaseUntil = 0;
	m_keyboardReleaseActive = false;
	memset(m_keyboardReleaseOffsets, 0, sizeof(m_keyboardReleaseOffsets));

	if (hadActiveLease)
	{
		CDInput8DeviceProxy::RequestInputReset();
	}
}

bool CTargetManager::ShouldDrawPrompt()
{
	return (m_flags & OMPPlusProtocol::TargetFlagHidePrompt) == 0;
}

bool CTargetManager::IsDirectSelectContext()
{
	return (m_flags & OMPPlusProtocol::TargetFlagDirectSelect) != 0;
}

bool CTargetManager::IsSelectableRow(unsigned char rowType)
{
	return rowType == OMPPlusProtocol::TargetRowAction
		|| rowType == OMPPlusProtocol::TargetRowToggle
		|| rowType == OMPPlusProtocol::TargetRowDanger;
}

unsigned int CTargetManager::GetFirstSelectableOptionId()
{
	for (size_t i = 0; i < m_options.size(); ++i)
	{
		const sTargetOption& option = m_options[i];
		if (option.enabled && IsSelectableRow(option.rowType) && option.optionId != 0)
			return option.optionId;
	}
	return 0;
}

bool CTargetManager::SendDirectSelect()
{
	const unsigned int optionId = GetFirstSelectableOptionId();
	if (optionId == 0)
		return false;

	BeginKeyboardReleaseLease();
	SendSelect(optionId);
	return true;
}

bool CTargetManager::UseLeftAlignedRows()
{
	switch (m_layout)
	{
	case OMPPlusProtocol::TargetLayoutCategory:
	case OMPPlusProtocol::TargetLayoutDialog:
	case OMPPlusProtocol::TargetLayoutWide:
		return true;
	case OMPPlusProtocol::TargetLayoutCompact:
	case OMPPlusProtocol::TargetLayoutStandard:
	case OMPPlusProtocol::TargetLayoutMinimal:
		return false;
	default:
		break;
	}

	return m_targetType == OMPPlusProtocol::TargetTypeNPC
		|| m_targetType == OMPPlusProtocol::TargetTypeActor
		|| m_targetType == OMPPlusProtocol::TargetTypeHouse;
}

float CTargetManager::GetMenuWidth()
{
	unsigned char layout = m_layout;
	if (layout == OMPPlusProtocol::TargetLayoutAuto)
	{
		switch (m_targetType)
		{
		case OMPPlusProtocol::TargetTypeVehicle:
			layout = OMPPlusProtocol::TargetLayoutStandard;
			break;
		case OMPPlusProtocol::TargetTypeNPC:
		case OMPPlusProtocol::TargetTypeActor:
			layout = OMPPlusProtocol::TargetLayoutDialog;
			break;
		case OMPPlusProtocol::TargetTypeHouse:
			layout = OMPPlusProtocol::TargetLayoutWide;
			break;
		case OMPPlusProtocol::TargetTypeItem:
			layout = OMPPlusProtocol::TargetLayoutCompact;
			break;
		default:
			layout = OMPPlusProtocol::TargetLayoutStandard;
			break;
		}
	}

	switch (layout)
	{
	case OMPPlusProtocol::TargetLayoutCompact:
		return 212.0f;
	case OMPPlusProtocol::TargetLayoutDialog:
		return 318.0f;
	case OMPPlusProtocol::TargetLayoutWide:
		return 350.0f;
	case OMPPlusProtocol::TargetLayoutMinimal:
		return 220.0f;
	case OMPPlusProtocol::TargetLayoutCategory:
		return 292.0f;
	case OMPPlusProtocol::TargetLayoutStandard:
	default:
		return 248.0f;
	}
}

float CTargetManager::GetHeaderHeight()
{
	if (m_layout == OMPPlusProtocol::TargetLayoutMinimal)
		return 26.0f;

	if (m_layout == OMPPlusProtocol::TargetLayoutCategory)
		return 32.0f;

	return HeaderHeight;
}

float CTargetManager::GetRowHeight(const sTargetOption& option)
{
	if (m_layout == OMPPlusProtocol::TargetLayoutCategory)
	{
		switch (option.rowType)
		{
		case OMPPlusProtocol::TargetRowHeader:
			return 22.0f;
		case OMPPlusProtocol::TargetRowInfo:
			return 30.0f;
		case OMPPlusProtocol::TargetRowDialog:
			return 56.0f;
		case OMPPlusProtocol::TargetRowDivider:
			return 8.0f;
		default:
			return 29.0f;
		}
	}

	switch (option.rowType)
	{
	case OMPPlusProtocol::TargetRowDialog:
		return DialogRowHeight;
	case OMPPlusProtocol::TargetRowInfo:
	case OMPPlusProtocol::TargetRowHeader:
		return InfoRowHeight;
	case OMPPlusProtocol::TargetRowDivider:
		return DividerRowHeight;
	default:
		return RowHeight;
	}
}

float CTargetManager::GetDescriptionHeight(float width)
{
	if (m_description.empty())
		return 0.0f;

	const float textWidth = (std::max)(1.0f, width - 24.0f);
	const float charsPerLine = (std::max)(12.0f, textWidth / 7.0f);
	const float lines = std::ceil(static_cast<float>(m_description.length()) / charsPerLine);
	return ClampFloat(lines * 17.0f + 8.0f, 26.0f, 82.0f);
}

float CTargetManager::GetMenuHeight(float width)
{
	float height = GetHeaderHeight() + RowGap + GetDescriptionHeight(width) + 34.0f;
	for (size_t i = 0; i < m_options.size(); ++i)
		height += GetRowHeight(m_options[i]) + RowGap;

	CPoint2D& resolution = CGraphics::GetScreenResolution();
	const float maxHeight = (std::max)(260.0f, static_cast<float>(resolution.Y()) - 80.0f);
	return ClampFloat(height, 80.0f, maxHeight);
}

void CTargetManager::OpenMenu()
{
	m_mouseSuppressUntil = 0;
	m_menuOpen = true;
	m_inputGuardUntil = GetTickCount() + 450;
	BeginKeyboardReleaseLease();
	CDInput8DeviceProxy::RequestInputReset();
	m_hoverIndex = -1;
	m_lastAltDown = (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
	m_lastEscapeDown = (GetAsyncKeyState(VK_ESCAPE) & 0x8000) != 0;
	m_lastRightDown = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
	m_lastLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	m_virtualCursorInitialized = false;
	m_cursorOwned = !CGraphics::IsCursorEnabled();
	CGraphics::ToggleCursor(true);
	SendMode(true);
}

void CTargetManager::CloseMenu(bool clearCursor)
{
	const bool wasOpen = m_menuOpen;
	if (m_menuOpen)
		SendMode(false);

	if (wasOpen)
		BeginKeyboardReleaseLease();

	m_menuOpen = false;
	m_inputGuardUntil = 0;
	m_hoverIndex = -1;
	m_lastLeftDown = false;
	m_virtualCursorInitialized = false;
	m_virtualWheel = 0;
	memset(m_mouseButtons, 0, sizeof(m_mouseButtons));
	if (wasOpen)
	{
		SuppressMouseInput(PostMouseSuppressMs);
		CDInput8DeviceProxy::RequestInputReset();
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

	const float cy = height * 0.5f;
	const float menuWidth = GetMenuWidth();
	const float totalHeight = GetMenuHeight(menuWidth);
	const float menuX = OverlayLayout::GetMenuX(width);
	const float menuY = cy - totalHeight * 0.5f;
	float rowY = menuY + GetHeaderHeight() + RowGap + GetDescriptionHeight(menuWidth);
	float cursorY = menuY + totalHeight * 0.5f;

	for (size_t i = 0; i < m_options.size(); ++i)
	{
		const sTargetOption& option = m_options[i];
		const float rowHeight = GetRowHeight(option);
		if (option.enabled && IsSelectableRow(option.rowType))
		{
			cursorY = rowY + rowHeight * 0.5f;
			break;
		}
		rowY += rowHeight + RowGap;
	}

	m_virtualCursorX = menuX + menuWidth * 0.5f;
	m_virtualCursorY = cursorY;
	m_virtualCursorInitialized = true;
	MoveWindowCursorToDisplayPoint(ImVec2(m_virtualCursorX, m_virtualCursorY));
}

void CTargetManager::ApplyImGuiInput()
{
	ImGuiIO& io = ImGui::GetIO();

	if (!m_virtualCursorInitialized)
		InitializeVirtualCursor();

	ImVec2 windowCursor;
	if (m_menuOpen && TryReadWindowCursor(windowCursor))
	{
		m_virtualCursorX = windowCursor.x;
		m_virtualCursorY = windowCursor.y;
	}

	const float maxX = (std::max)(1.0f, io.DisplaySize.x - 1.0f);
	const float maxY = (std::max)(1.0f, io.DisplaySize.y - 1.0f);

	m_virtualCursorX = (std::max)(0.0f, (std::min)(m_virtualCursorX, maxX));
	m_virtualCursorY = (std::max)(0.0f, (std::min)(m_virtualCursorY, maxY));

	io.MousePos = ImVec2(m_virtualCursorX, m_virtualCursorY);

	io.MouseDown[0] = m_menuOpen && (m_mouseButtons[0] || (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0);
	io.MouseDown[1] = m_menuOpen && (m_mouseButtons[1] || (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0);
	io.MouseDown[2] = m_menuOpen && (m_mouseButtons[2] || (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0);
	for (int i = 3; i < 5; ++i)
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
	ClearContextUnlocked();
}

void CTargetManager::UpdateHover()
{
	CPoint2D& resolution = CGraphics::GetScreenResolution();
	const float cy = static_cast<float>(resolution.Y()) * 0.5f;
	const float menuWidth = GetMenuWidth();
	const float totalHeight = GetMenuHeight(menuWidth);
	const float x = OverlayLayout::GetMenuX(static_cast<float>(resolution.X()));
	const float y = cy - totalHeight * 0.5f;
	float rowY = y + GetHeaderHeight() + RowGap + GetDescriptionHeight(menuWidth);

	if (m_menuOpen)
	{
		if (!m_virtualCursorInitialized)
			InitializeVirtualCursor();

		const float maxX = (std::max)(1.0f, static_cast<float>(resolution.X()) - 1.0f);
		const float maxY = (std::max)(1.0f, static_cast<float>(resolution.Y()) - 1.0f);
		m_virtualCursorX = (std::max)(0.0f, (std::min)(m_virtualCursorX, maxX));
		m_virtualCursorY = (std::max)(0.0f, (std::min)(m_virtualCursorY, maxY));
		m_cursor.x = static_cast<LONG>(m_virtualCursorX);
		m_cursor.y = static_cast<LONG>(m_virtualCursorY);
	}
	else
	{
		HWND hwnd = CMessageProxy::GetWindowHandle();
		GetCursorPos(&m_cursor);
		if (hwnd)
			ScreenToClient(hwnd, &m_cursor);
	}

	m_hoverIndex = -1;
	for (size_t i = 0; i < m_options.size(); ++i)
	{
		const sTargetOption& option = m_options[i];
		const float height = GetRowHeight(option);
		if (IsSelectableRow(option.rowType) && option.enabled && m_cursor.x >= x && m_cursor.x <= x + menuWidth && m_cursor.y >= rowY && m_cursor.y <= rowY + height)
		{
			m_hoverIndex = static_cast<int>(i);
			break;
		}
		rowY += height + RowGap;
	}
}

bool CTargetManager::ReadBoundString(RakNet::BitStream& bitStream, std::string& value, unsigned char maxLength)
{
	unsigned char length = 0;
	if (!bitStream.Read(length) || length > maxLength)
		return false;

	char buffer[160] = {};
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
	const D3DCOLOR accent = D3DCOLOR_ARGB(235, 245, 248, 246);
	const D3DCOLOR accentDim = D3DCOLOR_ARGB(92, 245, 248, 246);
	const D3DCOLOR black = D3DCOLOR_ARGB(205, 5, 7, 8);
	const D3DCOLOR blackLine = D3DCOLOR_ARGB(235, 0, 0, 0);

	DrawFilledRect(device, cx - 10.0f, cy - 10.0f, 20.0f, 20.0f, black);
	DrawOutlineRect(device, cx - 16.0f, cy - 13.0f, 32.0f, 26.0f, blackLine);
	DrawOutlineRect(device, cx - 12.0f, cy - 9.0f, 24.0f, 18.0f, accentDim);
	DrawFilledRect(device, cx - 4.0f, cy - 4.0f, 8.0f, 8.0f, accent);
	DrawLine(device, cx - 39.0f, cy, cx - 24.0f, cy, blackLine);
	DrawLine(device, cx + 24.0f, cy, cx + 39.0f, cy, blackLine);
	DrawLine(device, cx - 36.0f, cy, cx - 24.0f, cy, accentDim);
	DrawLine(device, cx + 24.0f, cy, cx + 36.0f, cy, accentDim);
}

void CTargetManager::DrawPromptLegend(IDirect3DDevice9* device, float cx, float cy)
{
	const std::string title = m_title.empty() ? "Target" : m_title;
	const float stackHeight = PromptTitleHeight + PromptLegendGap + PromptKeyHeight;
	const float stackTop = cy - stackHeight * 0.5f;
	const float width = (std::min)(PromptTitleMaxWidth, (std::max)(PromptTitleMinWidth, static_cast<float>(title.length()) * 7.5f + 18.0f));
	const float x = cx + PromptLegendOffsetX;
	const float y = stackTop;

	DrawFilledRect(device, x, y, width, PromptTitleHeight, D3DCOLOR_ARGB(220, 5, 7, 8));
	DrawOutlineRect(device, x, y, width, PromptTitleHeight, D3DCOLOR_ARGB(92, 245, 248, 246));
	DrawTextLine(title, static_cast<int>(x + 9.0f), static_cast<int>(y), static_cast<int>(width - 18.0f), static_cast<int>(PromptTitleHeight), D3DCOLOR_ARGB(220, 232, 234, 232));

	const float keyY = y + PromptTitleHeight + PromptLegendGap;
	DrawFilledRect(device, x, keyY, PromptKeyWidth, PromptKeyHeight, D3DCOLOR_ARGB(220, 5, 7, 8));
	DrawOutlineRect(device, x, keyY, PromptKeyWidth, PromptKeyHeight, D3DCOLOR_ARGB(92, 245, 248, 246));
	DrawTextLine("ALT", static_cast<int>(x + 9.0f), static_cast<int>(keyY), 30, static_cast<int>(PromptKeyHeight), D3DCOLOR_ARGB(220, 232, 234, 232));
}

void CTargetManager::DrawMenu(IDirect3DDevice9* device, float cx, float cy)
{
	const float menuWidth = GetMenuWidth();
	const float headerHeight = GetHeaderHeight();
	const float x = cx + OverlayLayout::MenuOffsetX;
	const float totalHeight = GetMenuHeight(menuWidth);
	const float y = cy - totalHeight * 0.5f;

	DrawFilledRect(device, x, y, menuWidth, headerHeight, D3DCOLOR_ARGB(205, 8, 10, 11));
	DrawOutlineRect(device, x, y, menuWidth, headerHeight, D3DCOLOR_ARGB(160, 235, 238, 235));
	DrawTextLine(m_title.empty() ? "Target" : m_title, static_cast<int>(x + 10.0f), static_cast<int>(y), static_cast<int>(menuWidth - 20.0f), static_cast<int>(headerHeight), D3DCOLOR_ARGB(255, 232, 255, 252));

	float rowY = y + headerHeight + RowGap;
	if (!m_description.empty())
	{
		const float descriptionHeight = GetDescriptionHeight(menuWidth);
		DrawFilledRect(device, x, rowY, menuWidth, descriptionHeight, D3DCOLOR_ARGB(160, 10, 12, 13));
		DrawTextLine(m_description, static_cast<int>(x + 10.0f), static_cast<int>(rowY), static_cast<int>(menuWidth - 20.0f), static_cast<int>(descriptionHeight), D3DCOLOR_ARGB(220, 210, 214, 210));
		rowY += descriptionHeight + RowGap;
	}

	for (size_t i = 0; i < m_options.size(); ++i)
	{
		const sTargetOption& option = m_options[i];
		const float rowHeight = GetRowHeight(option);
		if (option.rowType == OMPPlusProtocol::TargetRowDivider)
		{
			DrawLine(device, x + 10.0f, rowY + rowHeight * 0.5f, x + menuWidth - 10.0f, rowY + rowHeight * 0.5f, D3DCOLOR_ARGB(90, 235, 238, 235));
			rowY += rowHeight + RowGap;
			continue;
		}

		const bool hovered = static_cast<int>(i) == m_hoverIndex && IsSelectableRow(option.rowType);
		const bool selectable = option.enabled && IsSelectableRow(option.rowType);
		const D3DCOLOR bg = hovered ? D3DCOLOR_ARGB(230, 48, 50, 50) : D3DCOLOR_ARGB(205, 8, 10, 11);
		const D3DCOLOR border = hovered ? D3DCOLOR_ARGB(230, 245, 248, 246) : D3DCOLOR_ARGB(70, 235, 238, 235);
		const D3DCOLOR text = selectable ? D3DCOLOR_ARGB(255, 232, 255, 252) : D3DCOLOR_ARGB(175, 190, 196, 194);

		if (selectable || option.rowType == OMPPlusProtocol::TargetRowDisabled)
		{
			DrawFilledRect(device, x, rowY, menuWidth, rowHeight, bg);
			DrawOutlineRect(device, x, rowY, menuWidth, rowHeight, border);
		}

		std::string label = option.label;
		if (!option.icon.empty())
			label = option.icon + "  " + label;
		DrawTextLine(label, static_cast<int>(x + 10.0f), static_cast<int>(rowY), static_cast<int>(menuWidth - 20.0f), static_cast<int>(rowHeight), text);
		rowY += rowHeight + RowGap;
	}
}
