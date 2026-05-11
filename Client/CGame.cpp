#include <SAMP+/CRPC.h>
#include <SAMP+/client/Client.h>
#include <SAMP+/client/CHUD.h>
#include <SAMP+/client/CGame.h>
#include <SAMP+/client/CGraphics.h>
#include <SAMP+/client/CKeyBinds.h>
#include <SAMP+/client/CCmdlineParams.h>
#include <SAMP+/client/Proxy/CJmpProxy.h>
#include <SAMP+/client/Network.h>
#include <SAMP+/client/CSystem.h>
#include <SAMP+/client/CBuildManager.h>
#include <SAMP+/client/CTargetManager.h>

namespace
{
	const uintptr_t GtaPadBase = 0x00B73458;
	const uintptr_t GtaOldKeyState = 0x00B72F20;
	const uintptr_t GtaNewKeyState = 0x00B73190;
	const uintptr_t GtaPCTempMouseState = 0x00B73404;
	const uintptr_t GtaNewMouseState = 0x00B73418;
	const uintptr_t GtaOldMouseState = 0x00B7342C;
	const size_t GtaPadStride = 0x134;
	const unsigned int GtaPadCount = 2;
	const size_t CPadNewState = 0x00;
	const size_t CPadOldState = 0x30;
	const size_t CPadSteeringBuffer = 0x60;
	const size_t CPadDrunkDrivingBufferUsed = 0x74;
	const size_t CPadPCTempKeyState = 0x78;
	const size_t CPadPCTempJoyState = 0xA8;
	const size_t CPadPCTempMouseState = 0xD8;
	const size_t CPadDisablePlayerControls = 0x10E;
	const size_t ControllerStateSize = 0x30;
	const size_t KeyboardStateSize = 0x270;
	const size_t MouseStateSize = 0x14;
	const unsigned short TargetControlDisableBit = 0x8000;
	const unsigned int TargetTransitionFlushFrames = 2;

	bool g_targetControlBitApplied[GtaPadCount] = {};
	bool g_targetControlBitWasSet[GtaPadCount] = {};
	bool g_targetInputBlockWasActive = false;
	unsigned int g_targetTransitionFlushFrames = 0;

	typedef void*(__cdecl* GetPad_t)(int);
	typedef void(__thiscall* PadClear_t)(void*, bool, bool);
	typedef void(__cdecl* ClearMouseHistory_t)();

	GetPad_t GetPad = reinterpret_cast<GetPad_t>(0x53FB70);
	PadClear_t PadClear = reinterpret_cast<PadClear_t>(0x541A70);
	ClearMouseHistory_t ClearMouseHistory = reinterpret_cast<ClearMouseHistory_t>(0x541BD0);

	bool CanAccess(void* address, size_t size)
	{
		if (!address || !size)
			return false;

		MEMORY_BASIC_INFORMATION mbi = {};
		if (!VirtualQuery(address, &mbi, sizeof(mbi)))
			return false;

		if (mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			return false;

		const uintptr_t start = reinterpret_cast<uintptr_t>(address);
		const uintptr_t end = start + size;
		const uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
		return end <= regionEnd;
	}

	void ZeroPadRange(uintptr_t address, size_t size)
	{
		void* memory = reinterpret_cast<void*>(address);
		if (!CanAccess(memory, size))
			return;

		__try
		{
			memset(memory, 0, size);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	void ApplyPadControlBit(unsigned int padIndex, bool blocked)
	{
		if (padIndex >= GtaPadCount)
			return;

		unsigned short* disableControls = reinterpret_cast<unsigned short*>(GtaPadBase + (GtaPadStride * padIndex) + CPadDisablePlayerControls);
		if (!CanAccess(disableControls, sizeof(*disableControls)))
			return;

		__try
		{
			if (blocked)
			{
				if (!g_targetControlBitApplied[padIndex])
				{
					g_targetControlBitWasSet[padIndex] = ((*disableControls & TargetControlDisableBit) != 0);
					g_targetControlBitApplied[padIndex] = true;
				}
				*disableControls = static_cast<unsigned short>(*disableControls | TargetControlDisableBit);
			}
			else if (g_targetControlBitApplied[padIndex])
			{
				if (!g_targetControlBitWasSet[padIndex])
					*disableControls = static_cast<unsigned short>(*disableControls & ~TargetControlDisableBit);
				g_targetControlBitApplied[padIndex] = false;
				g_targetControlBitWasSet[padIndex] = false;
			}
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	void NeutralizePadInput(unsigned int padIndex)
	{
		const uintptr_t pad = GtaPadBase + (GtaPadStride * padIndex);
		const size_t controllerStateOffsets[] =
		{
			CPadNewState,
			CPadOldState,
			CPadPCTempKeyState,
			CPadPCTempJoyState,
			CPadPCTempMouseState
		};

		for (unsigned int i = 0; i < sizeof(controllerStateOffsets) / sizeof(controllerStateOffsets[0]); ++i)
			ZeroPadRange(pad + controllerStateOffsets[i], ControllerStateSize);

		ZeroPadRange(pad + CPadSteeringBuffer, 10 * sizeof(short));
		ZeroPadRange(pad + CPadDrunkDrivingBufferUsed, sizeof(int));
	}

	void NeutralizeGlobalInput()
	{
		ZeroPadRange(GtaOldKeyState, KeyboardStateSize);
		ZeroPadRange(GtaNewKeyState, KeyboardStateSize);
		ZeroPadRange(GtaPCTempMouseState, MouseStateSize);
		ZeroPadRange(GtaNewMouseState, MouseStateSize);
		ZeroPadRange(GtaOldMouseState, MouseStateSize);
	}

	void ClearPadOfficial(unsigned int padIndex)
	{
		__try
		{
			void* pad = GetPad(static_cast<int>(padIndex));
			if (pad && CanAccess(pad, GtaPadStride))
				PadClear(pad, true, true);
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}

	void ClearTargetInputFrame()
	{
		__try
		{
			for (unsigned int i = 0; i < GtaPadCount; ++i)
			{
				ClearPadOfficial(i);
				NeutralizePadInput(i);
				ApplyPadControlBit(i, false);
			}

			NeutralizeGlobalInput();
			ClearMouseHistory();
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
}

bool CGame::m_bGameLoaded;
bool CGame::InPauseMenu;
bool CGame::PauseMenuEnabled;
bool CGame::Frozen;
bool CGame::PedAnims;
bool CGame::RecreateMarkers = false;
bool CGame::VehicleBlips = true;
float CGame::AircraftMaxHeight;
bool CGame::inWater = false;

int CGame::X, CGame::Y;

int CGame::ClipAmmo[50];


void CGame::OnInitialize(IDirect3D9* pDirect3D, IDirect3DDevice9* pDevice, HWND hWindow)
{
	m_bGameLoaded = false;
	InPauseMenu = false;
	PauseMenuEnabled = true;

	X = 5;
	Y = 5;

//	box.Init(pDevice, 100, 60, 1366/2, 768/2, D3DCOLOR_ARGB(25, 255, 255, 255));

	CGraphics::Initialize(pDirect3D, pDevice);
}


void CGame::OnLoad()
{
	if (!CCmdlineParams::ArgumentExists("d") && CCmdlineParams::ArgumentExists("h"))
	{
		//CSystem::GetLoadedModules();
		CLog::Write("OnLoad success");
		CGame::UnprotectMemory();
		if (CCmdlineParams::ArgumentExists("sampp_legacy_sidechannel"))
			Network::Initialize(CCmdlineParams::GetArgumentValue("h"), atoi(CCmdlineParams::GetArgumentValue("p").c_str()) + 1);
		else
			Network::InitializeNative();
		Network::Connect();
		CHUD::Initialize();
		
		CGame::SetAircraftMaxHeight(800.0f);

		/*for (int i = 0; i < sizeof(50); ++i) {
			CGame::ClipAmmo[i] = 7;
		}*/
	}
}

void CGame::OnUnload()
{
	CGraphics::CleanUp();
}

int CGame::OnCursorMove(int iX, int iY)
{
	return !CTargetManager::IsMenuOpen() && !CBuildManager::ShouldBlockCursorMove();
}

void CGame::PreDeviceReset()
{
	CGraphics::OnReset();
}

BYTE CGame::OnPauseMenuChange(BYTE bFromMenuID, BYTE bToMenuID)
{
	RakNet::BitStream bitStream;
	bitStream.Write(bFromMenuID);
	bitStream.Write(bToMenuID);

	Network::SendRPC(eRPC::ON_PAUSE_MENU_CHANGE, &bitStream);

	return bToMenuID;
}

void CGame::PostDeviceReset()
{
	CGraphics::PostDeviceReset();
}

void CGame::PreEndScene()
{

	if(RecreateMarkers)
	{
		for(int i = 0; i < MAX_RACE_CHECKPOINTS; ++i)
		{
			char *thisCheckpoint = (char*) (RACE_CHECKPOINT_ADDR + (i * RACE_CHECKPOINT_OFFSET));

			if(*((unsigned short*) (thisCheckpoint)) != 257)
				*((bool*) (thisCheckpoint + 2)) = true;
		}

		RecreateMarkers = false;
	}

	CGraphics::PreEndScene();
}

void CGame::PostEndScene()
{

}

void CGame::BeginScene()
{
	CGraphics::BeginScene();
}

void CGame::OnWorldCreate()
{
	if (!IsLoaded())
	{
		OnLoad();
		m_bGameLoaded = true;
	}
}

void CGame::OnResolutionChange(UINT16 X, UINT16 Y)
{
	RakNet::BitStream bitStream;
	bitStream.Write(X);
	bitStream.Write(Y);

	Network::SendRPC(eRPC::ON_RESOLUTION_CHANGE, &bitStream);
}

void CGame::OnMouseClick(BYTE type, UINT16 X, UINT16 Y)
{
	RakNet::BitStream bitStream;
	bitStream.Write(type);
	bitStream.Write(X);
	bitStream.Write(Y);

	Network::SendRPC(eRPC::ON_MOUSE_CLICK, &bitStream);
}

void CGame::OnPauseMenuToggle(bool toggle)
{
	InPauseMenu = toggle;

	RakNet::BitStream bitStream;
	bitStream.Write(CGame::InPauseMenu);

	Network::SendRPC(eRPC::ON_PAUSE_MENU_TOGGLE, &bitStream);
}

void CGame::ToggleVehicleBlips(bool toggle)
{
	VehicleBlips = toggle;
}

bool CGame::IsLoaded()
{
	return m_bGameLoaded;
}

bool CGame::InMainMenu()
{
	return CMem::Get<bool>(0xBA6831);
}

bool CGame::Paused()
{
	return CMem::Get<bool>(0xB7CB49);
}

bool CGame::InMenu()
{
	return CMem::Get<bool>(0xBA67A4);
}

bool CGame::Playing()
{
	return IsLoaded() && !InMainMenu() && !Paused() && !InMenu();
}

bool CGame::HUDVisible()
{
	return CMem::Get<bool>(0xA444A0);
}

void CGame::SetHUDVisible(bool iVisible)
{
	CMem::PutSingle<bool>(0xA444A0, iVisible);
}

void CGame::SetRadioStation(unsigned long ulStation)
{
	((int(__thiscall*)(int, int, char, int, int))0x4EB3C0)(0x8CB6F8, ulStation, 0, 0, 0);
}

void CGame::SetWaveHeight(float fHeight)
{
	unsigned short ulOpcodes = 0xD8DD;
	CMem::Set(0x0072C667, 0x90, 0x04);
	CMem::Put<unsigned char>(0x72C665, &ulOpcodes, 2);
	CMem::Set(0x0072C659, 0x90, 0x0A);
	CMem::PutSingle<float>(0x00C812E8, fHeight);
}

void CGame::UnprotectMemory()
{
	CMem::Unprotect(0x72C659, 0x12);
	CMem::Unprotect(0xC812E8, sizeof(float));
	//CMem::Unprotect(0x00C7F158, 38 * NUM_CHECKPOINTS);
}

void CGame::SetBlurIntensity(int intensity) 
{
	CMem::PutSingle<BYTE>(0x0704E8A, 0xE8); // call
	CMem::PutSingle<BYTE>(0x0704E8B, 0x11);
	CMem::PutSingle<BYTE>(0x0704E8C, 0xE2);
	CMem::PutSingle<BYTE>(0x0704E8D, 0xFF);
	CMem::PutSingle<BYTE>(0x0704E8E, 0xFF);

	CMem::PutSingle<int>(0x8D5104, intensity);
}

void CGame::SetGameSpeed(float speed) 
{
	CMem::PutSingle<float>(0xB7CB64, speed);
}

void CGame::ToggleDriveOnWater(bool toggle) 
{
	CMem::PutSingle<BYTE>(0x969152, (BYTE)toggle);
}

void CGame::ToggleFrozen(bool toggle) 
{
	Frozen = toggle;
}

void CGame::SetPedAnims(bool toggle)
{
	PedAnims = toggle;
}

unsigned int CGame::IsFrozen() 
{
	return (unsigned int)Frozen;
}

unsigned int CGame::UsePedAnims()
{
	return (unsigned int)PedAnims;
}


void CGame::ToggleSwitchReload(bool toggle)
{
	if (!toggle)
	{
		memcpy((void*)0x060B4FA, "\x90\x90\x90\x90\x90\x90", 6);
	}
	else
	{
		memcpy((void*)0x060B4FA, "\x89\x88\xA8\x05\x00\x00", 6); // mov [eax+000005A8],ecx
	}
}

BYTE CGame::GetVersion()
{
	unsigned char ucA = CMem::Get<BYTE>(0x748ADD);
	unsigned char ucB = CMem::Get<BYTE>(0x748ADE);
	if (ucA == 0xFF && ucB == 0x53)
		return 1; // usa
	else if (ucA == 0x0F && ucB == 0x84)
		return 2; // eu
	else if (ucA == 0xFE && ucB == 0x10)
		return 3;

	return 0;
}

void CGame::SetAircraftMaxHeight(float height)
{
	AircraftMaxHeight = height;
}

void CGame::SetJetpackMaxHeight(float height)
{
	CMem::PutSingle<float>(0x8703D8, height);
}

void CGame::OnRadioChange(BYTE id)
{
	RakNet::BitStream bitStream;
	
	bitStream.Write(id);

	Network::SendRPC(eRPC::ON_RADIO_CHANGE, &bitStream);
}

void CGame::OnDrinkSprunk()
{
	Network::SendRPC(eRPC::ON_DRINK_SPRUNK);
}

void CGame::ToggleInfiniteOxygen(bool toggle)
{
	CMem::PutSingle<BYTE>(0x96916E, (BYTE)toggle);
}

void CGame::ToggleWaterBuoyancy(bool toggle)
{
	CMem::PutSingle <BYTE>(0x6C3F80, (BYTE) toggle);
	if (toggle)
	{
		memcpy((void*) 0x6C3F80, "\x90\x90\x90\x90\x90\x90\x90", 7); //NOP
	}
	else
	{
		memcpy((void*) 0x6C3F80, "\xC6\x87\x98\x00\x00\x00\x01", 7); //MOV BYTE PTR DS:[EDI+98],1
	}
}

void CGame::ToggleUnderwaterEffect(bool toggle)
{
	CMem::PutSingle<BYTE>(0x00C402D3, (BYTE) toggle);
}

void CGame::ToggleNightVision(bool toggle)
{
	CMem::PutSingle<BYTE>(0xC402B8, (BYTE) toggle);
}

void CGame::ToggleThermalVision(bool toggle)
{
	CMem::PutSingle<BYTE>(0xC402B9, (BYTE) toggle);
}

void CGame::ApplyTargetInputBlock()
{
	const bool active = CTargetManager::ShouldBlockGameControls() || CBuildManager::ShouldBlockGameControls();

	if (active != g_targetInputBlockWasActive)
	{
		g_targetTransitionFlushFrames = TargetTransitionFlushFrames;
		g_targetInputBlockWasActive = active;
	}

	if (active)
	{
		ClearTargetInputFrame();
		return;
	}

	if (g_targetTransitionFlushFrames > 0)
	{
		ClearTargetInputFrame();
		--g_targetTransitionFlushFrames;
		return;
	}

	for (unsigned int i = 0; i < GtaPadCount; ++i)
		ApplyPadControlBit(i, false);
}

void CGame::OnEnterWater()
{
	inWater = true;

	//Network::SendRPC(eRPC::ON_ENTER_WATER);
}

void CGame::OnLeaveWater()
{
	inWater = false;

	//Network::SendRPC(eRPC::ON_LEAVE_WATER);
}

void CGame::Present()
{
	//CGraphics::DrawString("Running SA-MP+ Pre-Alpha Release", 15, X, Y, D3DCOLOR_ARGB(255, 255, 255, 255));

	//CGraphics::DrawBox(50, 50, 100, 90, D3DCOLOR_ARGB(255, 255, 255, 255));
}
