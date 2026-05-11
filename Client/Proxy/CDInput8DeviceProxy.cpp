#include <SAMP+/client/Client.h>
#include <SAMP+/client/CBuildManager.h>
#include <SAMP+/client/CGraphics.h>
#include <SAMP+/client/CKeyBinds.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Proxy/CDInput8DeviceProxy.h>

namespace
{
	DWORD g_keyboardReleaseCursor = 0;
	DWORD g_mouseReleaseCursor = 0;
	volatile LONG g_inputResetGeneration = 0;

	const DWORD MouseReleaseOffsets[] =
	{
		DIMOFS_BUTTON0, DIMOFS_BUTTON1, DIMOFS_BUTTON2, DIMOFS_BUTTON3
	};

	DWORD WriteReleaseEvents(LPDIDEVICEOBJECTDATA data, DWORD capacity, const DWORD* offsets, DWORD offsetCount, DWORD& cursor)
	{
		if (!data || !capacity || !offsets || !offsetCount)
			return 0;

		const DWORD eventCount = capacity < offsetCount ? capacity : offsetCount;
		const DWORD now = GetTickCount();
		for (DWORD i = 0; i < eventCount; ++i)
		{
			const DWORD offset = offsets[(cursor + i) % offsetCount];
			data[i].dwOfs = offset;
			data[i].dwData = 0;
			data[i].dwTimeStamp = now;
			data[i].dwSequence = 0;
			data[i].uAppData = 0;
		}

		cursor = (cursor + eventCount) % offsetCount;
		return eventCount;
	}
}

CDInput8DeviceProxy::CDInput8DeviceProxy(IDirectInput8A* pInput, IDirectInputDevice8A* pDevice)
{
	m_pInput = pInput;
    m_pDevice = pDevice;
	m_dwType = 0;
	m_lResetGeneration = 0;

	DIDEVICEINSTANCEA di;
	di.dwSize = sizeof(di);
	if(SUCCEEDED(GetDeviceInfo(&di)))
		m_dwType = di.dwDevType & 0xff;

}

CDInput8DeviceProxy::~CDInput8DeviceProxy() { }

void CDInput8DeviceProxy::RequestInputReset()
{
	InterlockedIncrement(&g_inputResetGeneration);
}

void CDInput8DeviceProxy::ResetIfRequested()
{
	if (!m_pDevice || (m_dwType != DI8DEVTYPE_KEYBOARD && m_dwType != DI8DEVTYPE_MOUSE))
		return;

	const LONG requested = g_inputResetGeneration;
	if (m_lResetGeneration == requested)
		return;

	m_pDevice->Unacquire();
	m_pDevice->Acquire();
	FlushBufferedData();
	m_lResetGeneration = requested;
}

void CDInput8DeviceProxy::FlushBufferedData()
{
	if (!m_pDevice)
		return;

	DIDEVICEOBJECTDATA events[32] = {};
	for (unsigned int i = 0; i < 8; ++i)
	{
		DWORD count = sizeof(events) / sizeof(events[0]);
		const HRESULT hr = m_pDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), events, &count, 0);
		if (FAILED(hr) || count == 0)
			break;
	}
}

HRESULT CDInput8DeviceProxy::QueryInterface(REFIID riid,  LPVOID * ppvObj)
{
    return m_pDevice->QueryInterface(riid, ppvObj);
}

ULONG CDInput8DeviceProxy::AddRef(VOID)
{
    return m_pDevice->AddRef();
}

ULONG CDInput8DeviceProxy::Release(VOID)
{
	ULONG ulRefCount = m_pDevice->Release();
    if(ulRefCount == 0)
		delete this;

	return ulRefCount;
}

HRESULT CDInput8DeviceProxy::GetCapabilities(LPDIDEVCAPS a)
{
	return m_pDevice->GetCapabilities(a);
}

HRESULT CDInput8DeviceProxy::EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA a, LPVOID b, DWORD c)
{
    return m_pDevice->EnumObjects(a, b, c);
}

HRESULT CDInput8DeviceProxy::GetProperty(REFGUID a, LPDIPROPHEADER b)
{
    return m_pDevice->GetProperty(a, b);
}

HRESULT CDInput8DeviceProxy::SetProperty(REFGUID a, LPCDIPROPHEADER b)
{
    return m_pDevice->SetProperty(a, b);
}

HRESULT CDInput8DeviceProxy::Acquire(VOID)
{
    return m_pDevice->Acquire();
}

HRESULT CDInput8DeviceProxy::Unacquire(VOID)
{
    return m_pDevice->Unacquire();
}

HRESULT CDInput8DeviceProxy::GetDeviceState(DWORD a, LPVOID b)
{
	ResetIfRequested();

	if (DINPUT_DEVICE_IS_MOUSE && (CTargetManager::ShouldCaptureMouse() || CBuildManager::ShouldCaptureMouse()))
	{
		HRESULT hRes;

		hRes = m_pDevice->GetDeviceState(a, b);
		if (SUCCEEDED(hRes) && b)
		{
			if (CTargetManager::ShouldCaptureMouse())
				CTargetManager::FilterMouseState(a, b);
			else
				CBuildManager::FilterMouseState(a, b);
		}

		return hRes;
	}

	HRESULT hRes = m_pDevice->GetDeviceState(a, b);
	if (SUCCEEDED(hRes) && DINPUT_DEVICE_IS_KEYBOARD)
	{
		CKeyBinds::FilterKeyboardState(a, b);
		CTargetManager::FilterKeyboardState(a, b);
		CBuildManager::FilterKeyboardState(a, b);
	}

    return hRes;
}

HRESULT CDInput8DeviceProxy::GetDeviceData(DWORD a, LPDIDEVICEOBJECTDATA b, LPDWORD c, DWORD d)
{
	ResetIfRequested();

	const DWORD requestedItems = c ? *c : 0;
	HRESULT hRes = m_pDevice->GetDeviceData(a, b, c, d);

	if (SUCCEEDED(hRes) && DINPUT_DEVICE_IS_MOUSE && (CTargetManager::ShouldCaptureMouse() || CBuildManager::ShouldCaptureMouse()))
	{
		const bool targetCapture = CTargetManager::ShouldCaptureMouse();
		const bool buildPlacementCapture = !targetCapture && CBuildManager::ShouldPassMouseMovement();
		if (!c)
			return hRes;
		if (!b)
		{
			*c = 0;
			return hRes;
		}

		if ((d & DIGDD_PEEK) != 0)
		{
			*c = 0;
			return hRes;
		}

		DWORD writeIndex = 0;
		for (DWORD readIndex = 0; readIndex < *c; ++readIndex)
		{
			bool passEventToGame = false;
			switch (b[readIndex].dwOfs)
			{
				case DIMOFS_X:
					if (targetCapture)
						CTargetManager::AddMouseDelta(static_cast<LONG>(b[readIndex].dwData), 0, 0);
					else
						CBuildManager::AddMouseDelta(static_cast<LONG>(b[readIndex].dwData), 0, 0);
					passEventToGame = buildPlacementCapture;
					break;
				case DIMOFS_Y:
					if (targetCapture)
						CTargetManager::AddMouseDelta(0, static_cast<LONG>(b[readIndex].dwData), 0);
					else
						CBuildManager::AddMouseDelta(0, static_cast<LONG>(b[readIndex].dwData), 0);
					passEventToGame = buildPlacementCapture;
					break;
				case DIMOFS_Z:
					if (targetCapture)
						CTargetManager::AddMouseDelta(0, 0, static_cast<LONG>(b[readIndex].dwData));
					else
						CBuildManager::AddMouseDelta(0, 0, static_cast<LONG>(b[readIndex].dwData));
					break;
				case DIMOFS_BUTTON0:
				case DIMOFS_BUTTON1:
				case DIMOFS_BUTTON2:
				case DIMOFS_BUTTON3:
					if (targetCapture)
						CTargetManager::SetMouseButton((b[readIndex].dwOfs - DIMOFS_BUTTON0) / (DIMOFS_BUTTON1 - DIMOFS_BUTTON0), (b[readIndex].dwData & 0x80) != 0);
					else
						CBuildManager::SetMouseButton((b[readIndex].dwOfs - DIMOFS_BUTTON0) / (DIMOFS_BUTTON1 - DIMOFS_BUTTON0), (b[readIndex].dwData & 0x80) != 0);
					break;
				default:
					passEventToGame = buildPlacementCapture;
					break;
			}

			if (passEventToGame)
			{
				if (writeIndex != readIndex)
					b[writeIndex] = b[readIndex];
				++writeIndex;
			}
		}

		if (buildPlacementCapture)
		{
			*c = writeIndex;
			return hRes;
		}

		*c = WriteReleaseEvents(b, requestedItems, MouseReleaseOffsets, sizeof(MouseReleaseOffsets) / sizeof(MouseReleaseOffsets[0]), g_mouseReleaseCursor);
		return hRes;
	}

	if (SUCCEEDED(hRes) && DINPUT_DEVICE_IS_KEYBOARD && c)
	{
		if (CTargetManager::ShouldNeutralizeKeyboard() || CBuildManager::ShouldNeutralizeKeyboard())
		{
			if (!b)
			{
				*c = 0;
				return hRes;
			}
			if ((d & DIGDD_PEEK) != 0)
			{
				*c = 0;
				return hRes;
			}
			DWORD offsets[32] = {};
			const DWORD offsetCount = CTargetManager::ShouldNeutralizeKeyboard()
				? CTargetManager::GetKeyboardReleaseOffsets(offsets, sizeof(offsets) / sizeof(offsets[0]))
				: CBuildManager::GetKeyboardReleaseOffsets(offsets, sizeof(offsets) / sizeof(offsets[0]));
			*c = WriteReleaseEvents(b, requestedItems, offsets, offsetCount, g_keyboardReleaseCursor);
			return hRes;
		}

		DWORD writeIndex = 0;
		for (DWORD readIndex = 0; readIndex < *c; ++readIndex)
		{
			if (CKeyBinds::ShouldConsumeDirectInputOffset(b[readIndex].dwOfs)
				|| CTargetManager::ShouldConsumeDirectInputEvent(b[readIndex].dwOfs, b[readIndex].dwData))
				continue;
			if (CBuildManager::ShouldConsumeDirectInputEvent(b[readIndex].dwOfs, b[readIndex].dwData))
				continue;

			if (writeIndex != readIndex)
				b[writeIndex] = b[readIndex];
			++writeIndex;
		}
		*c = writeIndex;
	}

    return hRes;

	/*if (DINPUT_DEVICE_IS_MOUSE && CGraphics::IsCursorEnabled())
	{
		DWORD dwItems = INFINITE;
		HRESULT hRes;

		hRes = m_pDevice->GetDeviceData(a, b, c, d);

		if(d != DIGDD_PEEK)
			m_pDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &dwItems, 0);

		memset(b, 0, a*(*c));
		return hRes;
	}*/
}

HRESULT CDInput8DeviceProxy::SetDataFormat(LPCDIDATAFORMAT a)
{
    return m_pDevice->SetDataFormat(a);
}

HRESULT CDInput8DeviceProxy::SetEventNotification(HANDLE a)
{
    return m_pDevice->SetEventNotification(a);
}

HRESULT CDInput8DeviceProxy::SetCooperativeLevel(HWND a, DWORD b)
{
	return m_pDevice->SetCooperativeLevel(a, b);
}

HRESULT CDInput8DeviceProxy::GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA a, DWORD b, DWORD c)
{
    return m_pDevice->GetObjectInfo(a, b, c);
}

HRESULT CDInput8DeviceProxy::GetDeviceInfo(LPDIDEVICEINSTANCEA a)
{
    return m_pDevice->GetDeviceInfo(a);
}

HRESULT CDInput8DeviceProxy::RunControlPanel(HWND a, DWORD b)
{
    return m_pDevice->RunControlPanel(a, b);
}

HRESULT CDInput8DeviceProxy::Initialize(HINSTANCE a, DWORD b, REFGUID c)
{
    return m_pDevice->Initialize(a, b, c);
}

HRESULT CDInput8DeviceProxy::CreateEffect(REFGUID a, LPCDIEFFECT b, LPDIRECTINPUTEFFECT * c, LPUNKNOWN d)
{
    return m_pDevice->CreateEffect(a, b, c, d);
}

HRESULT CDInput8DeviceProxy::EnumEffects(LPDIENUMEFFECTSCALLBACKA a, LPVOID b, DWORD c)
{
    return m_pDevice->EnumEffects(a, b, c);
}

HRESULT CDInput8DeviceProxy::GetEffectInfo(LPDIEFFECTINFOA a, REFGUID b)
{
    return m_pDevice->GetEffectInfo(a, b);
}

HRESULT CDInput8DeviceProxy::GetForceFeedbackState(LPDWORD a)
{
    return m_pDevice->GetForceFeedbackState(a);
}

HRESULT CDInput8DeviceProxy::SendForceFeedbackCommand(DWORD a)
{
    return m_pDevice->SendForceFeedbackCommand(a);
}

HRESULT CDInput8DeviceProxy::EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK a, LPVOID b, DWORD c)
{
    return m_pDevice->EnumCreatedEffectObjects(a, b, c);
}

HRESULT CDInput8DeviceProxy::Escape(LPDIEFFESCAPE a)
{
    return m_pDevice->Escape(a);
}

HRESULT CDInput8DeviceProxy::Poll(VOID)
{
    return m_pDevice->Poll();
}

HRESULT CDInput8DeviceProxy::SendDeviceData(DWORD a, LPCDIDEVICEOBJECTDATA b, LPDWORD c, DWORD d)
{
    return m_pDevice->SendDeviceData(a, b, c, d);
}

HRESULT CDInput8DeviceProxy::EnumEffectsInFile(LPCSTR a, LPDIENUMEFFECTSINFILECALLBACK b, LPVOID c, DWORD d)
{
    return m_pDevice->EnumEffectsInFile(a, b, c, d);
}

HRESULT CDInput8DeviceProxy::WriteEffectToFile(LPCSTR a, DWORD b, LPDIFILEEFFECT c, DWORD d)
{
    return m_pDevice->WriteEffectToFile(a, b, c, d);
}

HRESULT CDInput8DeviceProxy::BuildActionMap(LPDIACTIONFORMATA a, LPCSTR b, DWORD c)
{
    return m_pDevice->BuildActionMap(a, b, c);
}

HRESULT CDInput8DeviceProxy::SetActionMap(LPDIACTIONFORMATA a, LPCSTR b, DWORD c)
{
    return m_pDevice->SetActionMap(a, b, c);
}

HRESULT CDInput8DeviceProxy::GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA a)
{
    return m_pDevice->GetImageInfo(a);
}
