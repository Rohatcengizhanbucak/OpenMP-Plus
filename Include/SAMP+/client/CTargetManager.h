#pragma once

#include <string>
#include <vector>

#include <DirectX/d3d9.h>
#include <DirectX/d3dx9.h>
#include <RakNet/BitStream.h>

struct sTargetOption
{
	unsigned int optionId;
	bool enabled;
	std::string label;
	std::string icon;
};

class CTargetManager
{
public:
	static void HandleSetContext(RakNet::BitStream& bitStream);
	static void ClearContext();
	static void Process();
	static void Draw(IDirect3DDevice9* device);
	static void OnDeviceInit(IDirect3DDevice9* device);
	static void OnLostDevice();
	static void OnResetDevice();
	static void CleanUp();
	static bool IsMenuOpen();
	static void RenderImGui();
	static bool HasActiveContext();
	static bool ShouldCaptureMouse();
	static bool ShouldSuppressKeyboard();
	static bool ShouldNeutralizeKeyboard();
	static bool ShouldBlockGameControls();
	static bool ShouldConsumeDirectInputOffset(DWORD offset);
	static bool ShouldConsumeDirectInputEvent(DWORD offset, DWORD data);
	static void FilterKeyboardState(DWORD size, LPVOID state);
	static void FilterMouseState(DWORD size, LPVOID state);
	static bool ShouldReleaseDirectInputOffset(DWORD offset);
	static DWORD GetKeyboardReleaseOffsets(DWORD* offsets, DWORD capacity);
	static void AddMouseDelta(LONG x, LONG y, LONG wheel = 0);
	static void SetMouseButton(unsigned int button, bool down);

private:
	static void ClearContextUnlocked();
	static bool HasValidContext();
	static bool InputAllowed();
	static bool IsMouseSuppressed();
	static bool HasKeyboardReleaseLease();
	static bool ShouldDrawPrompt();
	static void SuppressMouseInput(unsigned long durationMs);
	static void BeginKeyboardReleaseLease();
	static void UpdateKeyboardReleaseLease();
	static void ClearKeyboardReleaseLease();
	static void OpenMenu();
	static void CloseMenu(bool clearCursor);
	static void InitializeVirtualCursor();
	static void ApplyImGuiInput();
	static void SendMode(bool opened);
	static void SendSelect(unsigned int optionId);
	static void UpdateHover();
	static bool ReadBoundString(RakNet::BitStream& bitStream, std::string& value, unsigned char maxLength);
	static void EnsureFont(IDirect3DDevice9* device);
	static void DrawFilledRect(IDirect3DDevice9* device, float x, float y, float w, float h, D3DCOLOR colour);
	static void DrawOutlineRect(IDirect3DDevice9* device, float x, float y, float w, float h, D3DCOLOR colour);
	static void DrawLine(IDirect3DDevice9* device, float x1, float y1, float x2, float y2, D3DCOLOR colour);
	static void DrawTextLine(const std::string& text, int x, int y, int w, int h, D3DCOLOR colour);
	static void DrawEye(IDirect3DDevice9* device, float cx, float cy);
	static void DrawMenu(IDirect3DDevice9* device, float cx, float cy);

	static bool m_hasContext;
	static bool m_menuOpen;
	static bool m_cursorOwned;
	static bool m_lastAltDown;
	static bool m_lastLeftDown;
	static bool m_lastEscapeDown;
	static bool m_lastRightDown;
	static bool m_virtualCursorInitialized;
	static float m_virtualCursorX;
	static float m_virtualCursorY;
	static LONG m_virtualWheel;
	static bool m_mouseButtons[5];
	static unsigned int m_targetId;
	static unsigned int m_flags;
	static unsigned long m_expiresAt;
	static unsigned long m_lastContextTick;
	static unsigned long m_mouseSuppressUntil;
	static unsigned long m_keyboardReleaseUntil;
	static bool m_keyboardReleaseActive;
	static bool m_keyboardReleaseOffsets[256];
	static std::string m_title;
	static std::vector<sTargetOption> m_options;
	static int m_hoverIndex;
	static POINT m_cursor;
	static ID3DXFont* m_font;
};
