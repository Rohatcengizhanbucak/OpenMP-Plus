#pragma once

#include <Windows.h>
#include <DirectX/d3d9.h>

class COverlayRenderer
{
public:
	static void Enable();
	static void Disable();
	static bool TryInstall();
	static void Shutdown();
	static bool IsEnabled();
	static bool IsReady();
	static bool HasRenderHook();
	static bool HandleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	static void Render(IDirect3DDevice9* device);
	static void InvalidateDeviceObjects();
	static void RestoreDeviceObjects();
	static bool ValidateDevice(IDirect3DDevice9* device);

private:
	static IDirect3DDevice9* FindGameDevice();
	static bool EnsureInitialized(IDirect3DDevice9* device);
};
