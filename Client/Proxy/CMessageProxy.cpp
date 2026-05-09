#include <SAMP+/client/Client.h>
#include <SAMP+/client/CGame.h>
#include <SAMP+/client/CGraphics.h>
#include <SAMP+/client/COverlayRenderer.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Proxy/CMessageProxy.h>
#include <windowsx.h>

HWND CMessageProxy::m_hWindowOrig;
WNDPROC CMessageProxy::m_wProcOrig;

void CMessageProxy::Initialize(HWND hWindow)
{
	if (!hWindow || m_hWindowOrig == hWindow)
		return;

	if (m_hWindowOrig && m_wProcOrig)
		Uninitialize();

	m_wProcOrig = (WNDPROC)SetWindowLongPtr(hWindow, GWLP_WNDPROC, (LONG_PTR)Process);
	m_hWindowOrig = hWindow;
}

HWND CMessageProxy::GetWindowHandle()
{
	return m_hWindowOrig;
}

void CMessageProxy::Uninitialize()
{
	if (m_hWindowOrig == NULL)
		return;

	SetWindowLongPtr(m_hWindowOrig, GWLP_WNDPROC, (LONG_PTR)m_wProcOrig);
	m_hWindowOrig = NULL;
	m_wProcOrig = NULL;
}

WNDPROC CMessageProxy::GetOriginalProcedure()
{
	return m_wProcOrig;
}

//TODO: use Process for something useful
LRESULT CALLBACK CMessageProxy::Process(HWND wnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
	if (COverlayRenderer::HandleWndProc(wnd, umsg, wparam, lparam))
		return 0;

	if (CGame::Playing())
	{
		UINT vKey = (UINT)wparam;
	
		switch (umsg)
		{
			case WM_MOUSEMOVE:
			{
				if (CTargetManager::ShouldCaptureMouse())
					return 0;
				break;
			}
			//case WM_SYSKEYDOWN:
			case WM_KEYDOWN:
			{
				if (vKey == VK_F2 && (GetKeyState(VK_SHIFT) & 0x8000))
				{
					CGraphics::ToggleCursor(!CGraphics::IsCursorEnabled());
					return 0;
				}
				break;
			}
	
			case WM_LBUTTONDOWN:
			{
				if (CTargetManager::ShouldCaptureMouse())
					return 0;
				CGame::OnMouseClick(0, (UINT16)GET_X_LPARAM(lparam), (UINT16)GET_Y_LPARAM(lparam));
				break;
			}
			case WM_RBUTTONDOWN:
			{
				if (CTargetManager::ShouldCaptureMouse())
					return 0;
				CGame::OnMouseClick(1, (UINT16) GET_X_LPARAM(lparam), (UINT16) GET_Y_LPARAM(lparam));
				break;
			}
			case WM_MBUTTONDOWN:
			{
				if (CTargetManager::ShouldCaptureMouse())
					return 0;
				CGame::OnMouseClick(2, (UINT16) GET_X_LPARAM(lparam), (UINT16) GET_Y_LPARAM(lparam));
				break;
			}
		}
	}

	return CallWindowProc(CMessageProxy::GetOriginalProcedure(), wnd, umsg, wparam, lparam);
}

BOOL CMessageProxy::OnSetCursorPos(int iX, int iY)
{
	return CGame::OnCursorMove(iX, iY);
}
