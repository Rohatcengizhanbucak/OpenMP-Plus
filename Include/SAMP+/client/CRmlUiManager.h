#pragma once

#include <Windows.h>
#include <DirectX/d3d9.h>
#include <RakNet/BitStream.h>

class CRmlUiManager
{
public:
	static void Initialize(HWND window, IDirect3DDevice9* device);
	static void Shutdown();
	static void Clear();
	static void Process();
	static void Render(IDirect3DDevice9* device);
	static void InvalidateDeviceObjects();
	static void RestoreDeviceObjects();
	static bool HandleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static bool ShouldCaptureMouse();
	static bool ShouldNeutralizeKeyboard();
	static bool ShouldSuppressKeyboard();
	static void FilterKeyboardState(DWORD size, LPVOID state);
	static void FilterMouseState(DWORD size, LPVOID state);
	static bool ShouldConsumeDirectInputEvent(DWORD offset, DWORD data);
	static void AddMouseDelta(LONG x, LONG y, LONG wheel = 0);

	static void HandleOpen(RakNet::BitStream& stream);
	static void HandleClose(RakNet::BitStream& stream);
	static void HandleCloseAll();
	static void HandleSetData(RakNet::BitStream& stream);
	static void HandleInventoryClear(RakNet::BitStream& stream);
	static void HandleInventorySetSlot(RakNet::BitStream& stream);
};
