#pragma once

#include <string>
#include <vector>

#include <Windows.h>
#include <DirectX/dinput.h>
#include <RakNet/BitStream.h>

struct sBuildPart
{
	unsigned int partId;
	int modelId;
	std::string name;
	std::string category;
	std::string cost;
};

struct sBuildRemoveTarget
{
	bool active;
	unsigned int partId;
	float distance;
	std::string label;
	unsigned long receivedAt;
};

class CBuildManager
{
public:
	static void HandleOpen(RakNet::BitStream& bitStream);
	static void HandleClose();
	static void HandleClearParts();
	static void HandleAddPart(RakNet::BitStream& bitStream);
	static void HandleResult(RakNet::BitStream& bitStream);
	static void HandleRemoveTarget(RakNet::BitStream& bitStream);
	static void Clear();
	static void Process();
	static void RenderImGui();

	static bool IsActive();
	static bool IsMenuOpen();
	static bool ShouldCaptureMouse();
	static bool ShouldPassMouseMovement();
	static bool ShouldBlockCursorMove();
	static bool ShouldSuppressKeyboard();
	static bool ShouldNeutralizeKeyboard();
	static bool ShouldBlockGameControls();
	static bool ShouldConsumeDirectInputEvent(DWORD offset, DWORD data);
	static void FilterKeyboardState(DWORD size, LPVOID state);
	static void FilterMouseState(DWORD size, LPVOID state);
	static DWORD GetKeyboardReleaseOffsets(DWORD* offsets, DWORD capacity);
	static void AddMouseDelta(LONG x, LONG y, LONG wheel = 0);
	static void SetWindowMousePosition(LONG x, LONG y);
	static void SetMouseButton(unsigned int button, bool down);

private:
	static void ClearUnlocked(bool restoreCursor);
	static bool ReadBoundString(RakNet::BitStream& bitStream, std::string& value, unsigned char maxLength);
	static void InitializeVirtualCursor();
	static void ApplyImGuiInput();
	static void RenderRemoveOverlay();
	static bool IsMouseOverMenu();
	static void ReturnToMenu();
	static void SendSelect(unsigned int partId);
	static void SendClearPreview();
	static void SendPreviewState();
	static void SendPlace();
	static void SendCancel();

	static bool m_active;
	static bool m_menuOpen;
	static bool m_cursorOwned;
	static bool m_virtualCursorInitialized;
	static bool m_lastLeftDown;
	static bool m_lastRightDown;
	static bool m_lastMiddleDown;
	static bool m_lastEscapeDown;
	static bool m_lastQDown;
	static bool m_lastEDown;
	static float m_virtualCursorX;
	static float m_virtualCursorY;
	static LONG m_virtualWheel;
	static bool m_mouseButtons[5];
	static unsigned int m_sessionId;
	static unsigned int m_selectedPartId;
	static float m_maxDistance;
	static int m_rotationStep;
	static bool m_flipped;
	static unsigned long m_inputGuardUntil;
	static unsigned long m_statusUntil;
	static bool m_statusSuccess;
	static sBuildRemoveTarget m_removeTarget;
	static std::string m_title;
	static std::string m_status;
	static std::vector<sBuildPart> m_parts;
	static float m_menuX;
	static float m_menuY;
	static float m_menuW;
	static float m_menuH;
};
