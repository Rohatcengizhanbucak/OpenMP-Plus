#include <SAMP+/client/Client.h>
#include <SAMP+/client/CHooks.h>
#include <SAMP+/client/CGame.h>
#include <SAMP+/client/CLog.h>
#include <SAMP+/client/COverlayRenderer.h>
#include <SAMP+/client/CTargetManager.h>
#include <SAMP+/client/Proxy/CJmpProxy.h>

#include <Detours/detours.h>

namespace
{
	typedef short(__thiscall* PadShortGetter_t)(void*);
	typedef bool(__thiscall* PadBoolGetter_t)(void*);
	typedef unsigned char(__thiscall* PadByteGetter_t)(void*);

	PadShortGetter_t g_pfnGetSteeringLeftRight = NULL;
	PadShortGetter_t g_pfnGetSteeringUpDown = NULL;
	PadShortGetter_t g_pfnGetPedWalkLeftRight = NULL;
	PadShortGetter_t g_pfnGetPedWalkUpDown = NULL;
	PadShortGetter_t g_pfnGetCarGunFired = NULL;
	PadShortGetter_t g_pfnCarGunJustDown = NULL;
	PadShortGetter_t g_pfnGetHandBrake = NULL;
	PadShortGetter_t g_pfnGetBrake = NULL;
	PadShortGetter_t g_pfnGetAccelerate = NULL;
	PadBoolGetter_t g_pfnGetExitVehicle = NULL;
	PadBoolGetter_t g_pfnExitVehicleJustDown = NULL;
	PadByteGetter_t g_pfnGetMeleeAttack = NULL;
	PadByteGetter_t g_pfnMeleeAttackJustDown = NULL;
	PadBoolGetter_t g_pfnGetAccelerateJustDown = NULL;
	PadBoolGetter_t g_pfnCycleWeaponLeftJustDown = NULL;
	PadBoolGetter_t g_pfnCycleWeaponRightJustDown = NULL;
	PadBoolGetter_t g_pfnGetTarget = NULL;
	PadBoolGetter_t g_pfnGetDuck = NULL;
	PadBoolGetter_t g_pfnDuckJustDown = NULL;
	PadBoolGetter_t g_pfnGetJump = NULL;
	PadBoolGetter_t g_pfnJumpJustDown = NULL;
	PadBoolGetter_t g_pfnGetSprint = NULL;
	PadBoolGetter_t g_pfnSprintJustDown = NULL;
	PadBoolGetter_t g_pfnCollectPickupJustDown = NULL;

	bool ShouldNeutralizePadControls()
	{
		return CTargetManager::ShouldBlockGameControls();
	}

	short __fastcall HOOK_GetSteeringLeftRight(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnGetSteeringLeftRight(pad); }
	short __fastcall HOOK_GetSteeringUpDown(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnGetSteeringUpDown(pad); }
	short __fastcall HOOK_GetPedWalkLeftRight(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnGetPedWalkLeftRight(pad); }
	short __fastcall HOOK_GetPedWalkUpDown(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnGetPedWalkUpDown(pad); }
	short __fastcall HOOK_GetCarGunFired(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnGetCarGunFired(pad); }
	short __fastcall HOOK_CarGunJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnCarGunJustDown(pad); }
	short __fastcall HOOK_GetHandBrake(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnGetHandBrake(pad); }
	short __fastcall HOOK_GetBrake(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnGetBrake(pad); }
	short __fastcall HOOK_GetAccelerate(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnGetAccelerate(pad); }
	bool __fastcall HOOK_GetExitVehicle(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnGetExitVehicle(pad); }
	bool __fastcall HOOK_ExitVehicleJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnExitVehicleJustDown(pad); }
	unsigned char __fastcall HOOK_GetMeleeAttack(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnGetMeleeAttack(pad); }
	unsigned char __fastcall HOOK_MeleeAttackJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? 0 : g_pfnMeleeAttackJustDown(pad); }
	bool __fastcall HOOK_GetAccelerateJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnGetAccelerateJustDown(pad); }
	bool __fastcall HOOK_CycleWeaponLeftJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnCycleWeaponLeftJustDown(pad); }
	bool __fastcall HOOK_CycleWeaponRightJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnCycleWeaponRightJustDown(pad); }
	bool __fastcall HOOK_GetTarget(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnGetTarget(pad); }
	bool __fastcall HOOK_GetDuck(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnGetDuck(pad); }
	bool __fastcall HOOK_DuckJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnDuckJustDown(pad); }
	bool __fastcall HOOK_GetJump(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnGetJump(pad); }
	bool __fastcall HOOK_JumpJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnJumpJustDown(pad); }
	bool __fastcall HOOK_GetSprint(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnGetSprint(pad); }
	bool __fastcall HOOK_SprintJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnSprintJustDown(pad); }
	bool __fastcall HOOK_CollectPickupJustDown(void* pad, void*) { return ShouldNeutralizePadControls() ? false : g_pfnCollectPickupJustDown(pad); }

	void ApplyPadControlGetters()
	{
		if (!g_pfnGetSteeringLeftRight)
			g_pfnGetSteeringLeftRight = (PadShortGetter_t)DetourFunction((PBYTE)0x53FB80, (BYTE*)HOOK_GetSteeringLeftRight);
		if (!g_pfnGetSteeringUpDown)
			g_pfnGetSteeringUpDown = (PadShortGetter_t)DetourFunction((PBYTE)0x53FBD0, (BYTE*)HOOK_GetSteeringUpDown);
		if (!g_pfnGetPedWalkLeftRight)
			g_pfnGetPedWalkLeftRight = (PadShortGetter_t)DetourFunction((PBYTE)0x53FC90, (BYTE*)HOOK_GetPedWalkLeftRight);
		if (!g_pfnGetPedWalkUpDown)
			g_pfnGetPedWalkUpDown = (PadShortGetter_t)DetourFunction((PBYTE)0x53FD30, (BYTE*)HOOK_GetPedWalkUpDown);
		if (!g_pfnGetCarGunFired)
			g_pfnGetCarGunFired = (PadShortGetter_t)DetourFunction((PBYTE)0x53FF90, (BYTE*)HOOK_GetCarGunFired);
		if (!g_pfnCarGunJustDown)
			g_pfnCarGunJustDown = (PadShortGetter_t)DetourFunction((PBYTE)0x53FFE0, (BYTE*)HOOK_CarGunJustDown);
		if (!g_pfnGetHandBrake)
			g_pfnGetHandBrake = (PadShortGetter_t)DetourFunction((PBYTE)0x540040, (BYTE*)HOOK_GetHandBrake);
		if (!g_pfnGetBrake)
			g_pfnGetBrake = (PadShortGetter_t)DetourFunction((PBYTE)0x540080, (BYTE*)HOOK_GetBrake);
		if (!g_pfnGetExitVehicle)
			g_pfnGetExitVehicle = (PadBoolGetter_t)DetourFunction((PBYTE)0x5400D0, (BYTE*)HOOK_GetExitVehicle);
		if (!g_pfnExitVehicleJustDown)
			g_pfnExitVehicleJustDown = (PadBoolGetter_t)DetourFunction((PBYTE)0x540120, (BYTE*)HOOK_ExitVehicleJustDown);
		if (!g_pfnGetMeleeAttack)
			g_pfnGetMeleeAttack = (PadByteGetter_t)DetourFunction((PBYTE)0x540340, (BYTE*)HOOK_GetMeleeAttack);
		if (!g_pfnMeleeAttackJustDown)
			g_pfnMeleeAttackJustDown = (PadByteGetter_t)DetourFunction((PBYTE)0x540390, (BYTE*)HOOK_MeleeAttackJustDown);
		if (!g_pfnGetAccelerate)
			g_pfnGetAccelerate = (PadShortGetter_t)DetourFunction((PBYTE)0x5403F0, (BYTE*)HOOK_GetAccelerate);
		if (!g_pfnGetAccelerateJustDown)
			g_pfnGetAccelerateJustDown = (PadBoolGetter_t)DetourFunction((PBYTE)0x540440, (BYTE*)HOOK_GetAccelerateJustDown);
		if (!g_pfnCycleWeaponLeftJustDown)
			g_pfnCycleWeaponLeftJustDown = (PadBoolGetter_t)DetourFunction((PBYTE)0x540610, (BYTE*)HOOK_CycleWeaponLeftJustDown);
		if (!g_pfnCycleWeaponRightJustDown)
			g_pfnCycleWeaponRightJustDown = (PadBoolGetter_t)DetourFunction((PBYTE)0x540640, (BYTE*)HOOK_CycleWeaponRightJustDown);
		if (!g_pfnGetTarget)
			g_pfnGetTarget = (PadBoolGetter_t)DetourFunction((PBYTE)0x540670, (BYTE*)HOOK_GetTarget);
		if (!g_pfnGetDuck)
			g_pfnGetDuck = (PadBoolGetter_t)DetourFunction((PBYTE)0x540700, (BYTE*)HOOK_GetDuck);
		if (!g_pfnDuckJustDown)
			g_pfnDuckJustDown = (PadBoolGetter_t)DetourFunction((PBYTE)0x540720, (BYTE*)HOOK_DuckJustDown);
		if (!g_pfnGetJump)
			g_pfnGetJump = (PadBoolGetter_t)DetourFunction((PBYTE)0x540750, (BYTE*)HOOK_GetJump);
		if (!g_pfnJumpJustDown)
			g_pfnJumpJustDown = (PadBoolGetter_t)DetourFunction((PBYTE)0x540770, (BYTE*)HOOK_JumpJustDown);
		if (!g_pfnGetSprint)
			g_pfnGetSprint = (PadBoolGetter_t)DetourFunction((PBYTE)0x5407A0, (BYTE*)HOOK_GetSprint);
		if (!g_pfnSprintJustDown)
			g_pfnSprintJustDown = (PadBoolGetter_t)DetourFunction((PBYTE)0x5407F0, (BYTE*)HOOK_SprintJustDown);
		if (!g_pfnCollectPickupJustDown)
			g_pfnCollectPickupJustDown = (PadBoolGetter_t)DetourFunction((PBYTE)0x540A70, (BYTE*)HOOK_CollectPickupJustDown);

		CLog::Write("CPad target control getter hooks installed");
	}

	void RemovePadControlGetters()
	{
		if (g_pfnGetSteeringLeftRight) { DetourRemove((BYTE*)g_pfnGetSteeringLeftRight, (BYTE*)HOOK_GetSteeringLeftRight); g_pfnGetSteeringLeftRight = NULL; }
		if (g_pfnGetSteeringUpDown) { DetourRemove((BYTE*)g_pfnGetSteeringUpDown, (BYTE*)HOOK_GetSteeringUpDown); g_pfnGetSteeringUpDown = NULL; }
		if (g_pfnGetPedWalkLeftRight) { DetourRemove((BYTE*)g_pfnGetPedWalkLeftRight, (BYTE*)HOOK_GetPedWalkLeftRight); g_pfnGetPedWalkLeftRight = NULL; }
		if (g_pfnGetPedWalkUpDown) { DetourRemove((BYTE*)g_pfnGetPedWalkUpDown, (BYTE*)HOOK_GetPedWalkUpDown); g_pfnGetPedWalkUpDown = NULL; }
		if (g_pfnGetCarGunFired) { DetourRemove((BYTE*)g_pfnGetCarGunFired, (BYTE*)HOOK_GetCarGunFired); g_pfnGetCarGunFired = NULL; }
		if (g_pfnCarGunJustDown) { DetourRemove((BYTE*)g_pfnCarGunJustDown, (BYTE*)HOOK_CarGunJustDown); g_pfnCarGunJustDown = NULL; }
		if (g_pfnGetHandBrake) { DetourRemove((BYTE*)g_pfnGetHandBrake, (BYTE*)HOOK_GetHandBrake); g_pfnGetHandBrake = NULL; }
		if (g_pfnGetBrake) { DetourRemove((BYTE*)g_pfnGetBrake, (BYTE*)HOOK_GetBrake); g_pfnGetBrake = NULL; }
		if (g_pfnGetExitVehicle) { DetourRemove((BYTE*)g_pfnGetExitVehicle, (BYTE*)HOOK_GetExitVehicle); g_pfnGetExitVehicle = NULL; }
		if (g_pfnExitVehicleJustDown) { DetourRemove((BYTE*)g_pfnExitVehicleJustDown, (BYTE*)HOOK_ExitVehicleJustDown); g_pfnExitVehicleJustDown = NULL; }
		if (g_pfnGetMeleeAttack) { DetourRemove((BYTE*)g_pfnGetMeleeAttack, (BYTE*)HOOK_GetMeleeAttack); g_pfnGetMeleeAttack = NULL; }
		if (g_pfnMeleeAttackJustDown) { DetourRemove((BYTE*)g_pfnMeleeAttackJustDown, (BYTE*)HOOK_MeleeAttackJustDown); g_pfnMeleeAttackJustDown = NULL; }
		if (g_pfnGetAccelerate) { DetourRemove((BYTE*)g_pfnGetAccelerate, (BYTE*)HOOK_GetAccelerate); g_pfnGetAccelerate = NULL; }
		if (g_pfnGetAccelerateJustDown) { DetourRemove((BYTE*)g_pfnGetAccelerateJustDown, (BYTE*)HOOK_GetAccelerateJustDown); g_pfnGetAccelerateJustDown = NULL; }
		if (g_pfnCycleWeaponLeftJustDown) { DetourRemove((BYTE*)g_pfnCycleWeaponLeftJustDown, (BYTE*)HOOK_CycleWeaponLeftJustDown); g_pfnCycleWeaponLeftJustDown = NULL; }
		if (g_pfnCycleWeaponRightJustDown) { DetourRemove((BYTE*)g_pfnCycleWeaponRightJustDown, (BYTE*)HOOK_CycleWeaponRightJustDown); g_pfnCycleWeaponRightJustDown = NULL; }
		if (g_pfnGetTarget) { DetourRemove((BYTE*)g_pfnGetTarget, (BYTE*)HOOK_GetTarget); g_pfnGetTarget = NULL; }
		if (g_pfnGetDuck) { DetourRemove((BYTE*)g_pfnGetDuck, (BYTE*)HOOK_GetDuck); g_pfnGetDuck = NULL; }
		if (g_pfnDuckJustDown) { DetourRemove((BYTE*)g_pfnDuckJustDown, (BYTE*)HOOK_DuckJustDown); g_pfnDuckJustDown = NULL; }
		if (g_pfnGetJump) { DetourRemove((BYTE*)g_pfnGetJump, (BYTE*)HOOK_GetJump); g_pfnGetJump = NULL; }
		if (g_pfnJumpJustDown) { DetourRemove((BYTE*)g_pfnJumpJustDown, (BYTE*)HOOK_JumpJustDown); g_pfnJumpJustDown = NULL; }
		if (g_pfnGetSprint) { DetourRemove((BYTE*)g_pfnGetSprint, (BYTE*)HOOK_GetSprint); g_pfnGetSprint = NULL; }
		if (g_pfnSprintJustDown) { DetourRemove((BYTE*)g_pfnSprintJustDown, (BYTE*)HOOK_SprintJustDown); g_pfnSprintJustDown = NULL; }
		if (g_pfnCollectPickupJustDown) { DetourRemove((BYTE*)g_pfnCollectPickupJustDown, (BYTE*)HOOK_CollectPickupJustDown); g_pfnCollectPickupJustDown = NULL; }
	}
}

Direct3DCreate9_t  CHooks::m_pfnDirect3DCreate9 = NULL;
DirectInput8Create_t CHooks::m_pfnDirectInput8Create = NULL;
SetCursorPos_t  CHooks::m_pfnSetCursorPos = NULL;
UpdatePads_t CHooks::m_pfnUpdatePads = NULL;

void CHooks::Apply()
{
	ApplyCursorPos();
	ApplyDirect3D();
	ApplyDirectInput();
	ApplyPadUpdate();
	InstallJmp();
	InstallPatches();
}

void CHooks::ApplySafeInput()
{
	ApplyDirectInput();
	ApplyPadUpdate();
}

void CHooks::ApplySafeGraphics()
{
	COverlayRenderer::Enable();
	ApplyCursorPos();
}

void CHooks::Remove()
{
	RemoveDirectInput();
	RemoveDirect3D();
	RemoveCursorPos();
	RemovePadUpdate();
	COverlayRenderer::Shutdown();
}

void CHooks::InstallJmp()
{
	CMem::InstallJmp(0x57377D, CJmpProxy::MenuAction1, CJmpProxy::MenuJumpBack1, 6);
	CMem::InstallJmp(0x5736CF, CJmpProxy::MenuAction2, CJmpProxy::MenuJumpBack2, 6);
	CMem::InstallJmp(0x57C2F7, CJmpProxy::MenuAction3, CJmpProxy::MenuJumpBack3, 6);
	CMem::InstallJmp(0x576C27, CJmpProxy::MenuSwitch, CJmpProxy::MenuSwitchJumpBack, 6, 8);
	CMem::InstallJmp(0x52B811, CJmpProxy::WorldCreate, CJmpProxy::WorldCreateJumpBack, 6);
	CMem::InstallJmp(0x58F4A2, CJmpProxy::PositiveMoneyDraw, CJmpProxy::PositiveMoneyDrawJumpBack, 8);
	//CMem::InstallJmp(0x58EBDA, CJmpProxy::UnknownMoneyDraw1, CJmpProxy::UnknownMoneyDraw1JumpBack, 8);
	CMem::InstallJmp(0x58F4E4, CJmpProxy::NegativeMoneyDraw, CJmpProxy::NegativeMoneyDrawJumpBack, 8);
	CMem::InstallJmp(0x589117, CJmpProxy::ArmourBarDraw, CJmpProxy::ArmourBarDrawJumpBack, 6);
	CMem::InstallJmp(0x589344, CJmpProxy::HealthBarDraw, CJmpProxy::HealthBarDrawJumpBack, 8);
	CMem::InstallJmp(0x589202, CJmpProxy::BreathBarDraw, CJmpProxy::BreathBarDrawJumpBack, 6);
	CMem::InstallJmp(0x589608, CJmpProxy::AmmoDraw, CJmpProxy::AmmoDrawJumpBack, 8);
	CMem::InstallJmp(0x58DDD9, CJmpProxy::WantedLevelDraw, CJmpProxy::WantedLevelDrawJumpBack, 8);
	CMem::InstallJmp(0x4E9FA1, CJmpProxy::ActiveRadioDraw);
	CMem::InstallJmp(0x4E9FB2, CJmpProxy::InactiveRadioDraw, CJmpProxy::InactiveRadioDrawJumpBack, 7);
	//TODO: unknown active draw (71A220)
	CMem::InstallJmp(0x60F583, CJmpProxy::DriveByUnknown, CJmpProxy::DriveByUnknownJumpBack, 6);
	CMem::InstallJmp(0x469941, CJmpProxy::StuntBonus, CJmpProxy::StuntBonusJumpBack, 6);
	CMem::InstallJmp(0x69E360, CJmpProxy::StuntInfo, CJmpProxy::StuntInfoJumpBack, 58);
	//CMem::InstallJmp(0x07F6CFD, CJmpProxy::ChangeResolution, CJmpProxy::ChangeResolutionJumpBack, 15);
	CMem::InstallJmp(0x0609560, CJmpProxy::FreezePed, CJmpProxy::FreezePedJumpBack, 6);
	CMem::InstallJmp(0x06B4CC0, CJmpProxy::FreezeVehicle, CJmpProxy::FreezeVehicleJumpBack, 6);
	CMem::InstallJmp(0x0609A1F, CJmpProxy::PedAnims, CJmpProxy::PedAnimsJumpBack, 7);
	CMem::InstallJmp(0x06D2612, CJmpProxy::AircraftMaxHeight1, CJmpProxy::AircraftMaxHeight1JumpBack, 6);
	CMem::InstallJmp(0x06D2623, CJmpProxy::AircraftMaxHeight2, CJmpProxy::AircraftMaxHeight2JumpBack, 6);
	//CMem::InstallJmp(0x060B4F4, CJmpProxy::SwitchWeapon, CJmpProxy::SwitchWeaponJumpBack, 6);

	CMem::InstallJmp(0x0584770, CJmpProxy::MarkersHook, CJmpProxy::MarkersHookJmpBack, 6);
	CMem::InstallJmp(0x4EB484, CJmpProxy::RadioHook, CJmpProxy::RadioHookJmpBack, 6);
	CMem::InstallJmp(0x55C330, CJmpProxy::DrinkSprunkHook, CJmpProxy::DrinkSprunkJmpBack, 6);
	
}

void CHooks::InstallPatches()
{
	CMem::Cpy((void*)0x0401190, "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 10); // NOP SetGameSpeed to 1.0 every frame
	CMem::Cpy((void*)0x060CD41, "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 10); // NOP SetGameSpeed to 1.0 on player death
}

void CHooks::ApplyDirectInput()
{
	if (!m_pfnDirectInput8Create)
	{
		PBYTE target = DetourFindFunction("dinput8.dll", "DirectInput8Create");
		if (!target)
		{
			CLog::Write("DirectInput capture hook unavailable");
			return;
		}
		m_pfnDirectInput8Create = (DirectInput8Create_t)DetourFunction(target, (BYTE*)CHooks::HOOK_DirectInput8Create);
	}
}

void CHooks::RemoveDirectInput()
{
	if (m_pfnDirectInput8Create)
	{
		DetourRemove((BYTE*)m_pfnDirectInput8Create, (BYTE*)HOOK_DirectInput8Create);
		m_pfnDirectInput8Create = NULL;
	}
}

void CHooks::ApplyDirect3D()
{
	if (!m_pfnDirect3DCreate9)
		m_pfnDirect3DCreate9 = (Direct3DCreate9_t)DetourFunction(DetourFindFunction("d3d9.dll", "Direct3DCreate9"), (BYTE*)HOOK_Direct3DCreate9);
}

void CHooks::RemoveDirect3D()
{
	if (m_pfnDirect3DCreate9)
	{
		DetourRemove((BYTE*)m_pfnDirect3DCreate9, (BYTE*)HOOK_Direct3DCreate9);
		m_pfnDirect3DCreate9 = NULL;
	}
}

void CHooks::ApplyCursorPos()
{
	if(!m_pfnSetCursorPos)
		m_pfnSetCursorPos = (SetCursorPos_t)DetourFunction(DetourFindFunction("user32.dll", "SetCursorPos"), (BYTE*)HOOK_SetCursorPos);
}

void CHooks::RemoveCursorPos()
{
	if (m_pfnSetCursorPos)
	{
		DetourRemove((BYTE*)m_pfnSetCursorPos, (BYTE*)HOOK_SetCursorPos);
		m_pfnSetCursorPos = NULL;
	}
}

void CHooks::ApplyPadUpdate()
{
	if (!m_pfnUpdatePads)
	{
		m_pfnUpdatePads = (UpdatePads_t)DetourFunction((PBYTE)0x541DD0, (BYTE*)HOOK_UpdatePads);
		if (m_pfnUpdatePads)
			CLog::Write("CPad target input hook installed");
		else
			CLog::Write("CPad target input hook unavailable");
	}
}

void CHooks::RemovePadUpdate()
{
	if (m_pfnUpdatePads)
	{
		DetourRemove((BYTE*)m_pfnUpdatePads, (BYTE*)HOOK_UpdatePads);
		m_pfnUpdatePads = NULL;
	}
}

HRESULT CHooks::HOOK_DirectInput8Create(HINSTANCE hinst, DWORD dwVersion, REFIID riidltf, void** ppvOut, IUnknown* punkOuter)
{
	HRESULT hr = m_pfnDirectInput8Create(hinst, dwVersion, riidltf, ppvOut, punkOuter);
	if (SUCCEEDED(hr) && ppvOut && *ppvOut)
	{
		IDirectInput8A* pDInput = (IDirectInput8A*)*ppvOut;
		CDInput8Proxy* pDInputHook = new CDInput8Proxy(pDInput);
		*ppvOut = pDInputHook;
	}
	return hr;
}

IDirect3D9* WINAPI CHooks::HOOK_Direct3DCreate9(UINT SDKVersion)
{
	IDirect3D9* Direct3D = m_pfnDirect3DCreate9(SDKVersion);
	IDirect3D9* Mine_Direct3D = new CD3D9Proxy(Direct3D);
	return Mine_Direct3D;
}

BOOL WINAPI CHooks::HOOK_SetCursorPos(int iX, int iY)
{
	if (CMessageProxy::OnSetCursorPos(iX, iY))
		m_pfnSetCursorPos(iX, iY);
		
	return false;
}

void __cdecl CHooks::HOOK_UpdatePads()
{
	m_pfnUpdatePads();
	CGame::ApplyTargetInputBlock();
}
