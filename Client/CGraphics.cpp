#include <SAMP+/client/CGraphics.h>
#include <SAMP+/client/COverlayRenderer.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Network.h>

CPoint2D CGraphics::m_pResolution;
IDirect3D9* CGraphics::m_pDirect3D;
IDirect3DDevice9* CGraphics::m_pDevice;
bool CGraphics::m_bCursorEnabled;

Sprite* CGraphics::logo;
Box* CGraphics::box;

VOID CGraphics::SetScreenResolution(UINT uiWidth, UINT uiHeight)
{
	m_pResolution.X() = uiWidth;
	m_pResolution.Y() = uiHeight;
}

CPoint2D& CGraphics::GetScreenResolution()
{
	return m_pResolution;
}


void CGraphics::Initialize(IDirect3D9* pDirect3D, IDirect3DDevice9* pDevice)
{
	m_pDevice = pDevice;
	m_pDirect3D = pDirect3D;
	m_bCursorEnabled = false;
	UpdateScreenResolution();

	CLog::Write("CGraphics::Initialize");
}

void CGraphics::AttachDevice(IDirect3DDevice9* pDevice)
{
	if (!pDevice)
		return;

	m_pDevice = pDevice;

	D3DVIEWPORT9 viewport = {};
	if (SUCCEEDED(m_pDevice->GetViewport(&viewport)) && viewport.Width && viewport.Height)
		SetScreenResolution(viewport.Width, viewport.Height);
}

IDirect3DDevice9* CGraphics::GetDevice()
{
	return m_pDevice;
}

void CGraphics::UpdateScreenResolution()
{
	if (!m_pDirect3D)
	{
		AttachDevice(m_pDevice);
		return;
	}

	D3DDISPLAYMODE dm;
	m_pDirect3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &dm);
	SetScreenResolution(dm.Width, dm.Height);
}

HRESULT CGraphics::ToggleCursor(bool toggle)
{
	m_bCursorEnabled = toggle;
	if (!m_pDevice)
		return S_OK;
	return m_pDevice->ShowCursor(toggle);
}

bool CGraphics::IsCursorEnabled()
{
	return m_bCursorEnabled;
}

void CGraphics::OnReset()
{
	CLog::Write("CGraphics::OnReset");
	CTargetManager::OnLostDevice();
}

void CGraphics::PostDeviceReset()
{
	CLog::Write("CGraphics::PostDeviceReset");
	CTargetManager::OnResetDevice();
}

void CGraphics::PreEndScene()
{
	if (m_pDevice && Network::IsConnected() && !COverlayRenderer::HasRenderHook())
		CTargetManager::Draw(m_pDevice);
}

void CGraphics::BeginScene()
{
	
}

void CGraphics::CleanUp()
{
	CLog::Write("CGraphics::CleanUp");
	if (logo)
	{
		delete logo;
		logo = nullptr;
	}
	if (box)
	{
		delete box;
		box = nullptr;
	}
	CTargetManager::CleanUp();
}
