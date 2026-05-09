#include <RakNet/BitStream.h>

#include <algorithm>
#include <string>

#include <SAMP+/Utility.h>
#include <SAMP+/CRPC.h>
#include <SAMP+/OMPPlusProtocol.h>
#include <SAMP+/svr/Plugin.h>
#include <SAMP+/svr/Callback.h>

#include <sampgdk.h>

static CPlayer* GetSAMPPPlayer(cell playerid, const char* nativeName)
{
	CPlayer* pPlayer = Network::GetPlayerFromPlayerid((unsigned int)playerid);

	if (!pPlayer)
		Utility::Printf("%s called for player %d without SA-MP+ client", nativeName, (int)playerid);

	return pPlayer;
}

static std::string GetPawnString(AMX* pAmx, cell amxAddress, size_t maxLength)
{
	cell* pString = NULL;
	if (amx_GetAddr(pAmx, amxAddress, &pString) != AMX_ERR_NONE || !pString)
		return "";

	int length = 0;
	if (amx_StrLen(pString, &length) != AMX_ERR_NONE || length <= 0)
		return "";

	if ((size_t)length > maxLength)
		length = (int)maxLength;

	char buffer[64] = { 0 };
	amx_GetString(buffer, pString, false, (size_t)length + 1);
	return std::string(buffer, (size_t)length);
}

cell AMX_NATIVE_CALL CallbackProc(AMX* pAmx, cell* pParams)
{	
	return Callback::Process(pAmx, (Callback::eCallbackType)pParams[1], &pParams[2]);
}

cell AMX_NATIVE_CALL ToggleHUDComponentForPlayerProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.WriteCasted<unsigned char, cell>(pParams[2]); // component
	bitStream.Write(!!pParams[3]); // toggle

	return Network::PlayerSendRPC(eRPC::TOGGLE_HUD_COMPONENT, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL SetPlayerHUDComponentColourProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.WriteCasted<unsigned char, cell>(pParams[2]); // colour component
	bitStream.WriteCasted<unsigned int, cell>(pParams[3]); // colour

	return Network::PlayerSendRPC(eRPC::SET_HUD_COMPONENT_COLOUR, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL SetRadioStationForPlayerProc(AMX* pAmx, cell* pParams)
{
	if (IsPlayerInAnyVehicle(pParams[1]))
	{
		RakNet::BitStream bitStream;
		bitStream.WriteCasted<unsigned char, cell>(pParams[2]); // radio station id

		return Network::PlayerSendRPC(eRPC::SET_RADIO_STATION, pParams[1], &bitStream);
	}

	return 1;
}

cell AMX_NATIVE_CALL GetRadioStation(AMX* pAmx, cell* pParams)
{
	if (IsPlayerInAnyVehicle(pParams[1]))
	{
		CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "GetPlayerRadioStation");
		return pPlayer ? pPlayer->GetRadio() : 0;
	}
	return 0;
}

cell AMX_NATIVE_CALL SetWaveHeightForPlayerProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(amx_ctof(pParams[2])); // wave height

	return Network::PlayerSendRPC(eRPC::SET_WAVE_HEIGHT, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL TogglePauseMenuAbilityProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]); // toggle

	return Network::PlayerSendRPC(eRPC::TOGGLE_PAUSE_MENU, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL ToggleInfiniteOxygen(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]); // toggle

	return Network::PlayerSendRPC(eRPC::TOGGLE_INFINITE_OXYGEN, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL TogglePlayerWaterBuoyancy(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]); // toggle

	return Network::PlayerSendRPC(eRPC::TOGGLE_WATER_BUOYANCY, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL ToggleUnderwaterEffect(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]); // toggle

	return Network::PlayerSendRPC(eRPC::TOGGLE_UNDERWATER_EFFECT, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL ToggleNightVision(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]); // toggle

	return Network::PlayerSendRPC(eRPC::TOGGLE_NIGHTVISION, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL ToggleThermalVision(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]); // toggle

	return Network::PlayerSendRPC(eRPC::TOGGLE_THERMALVISION, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL IsPlayerInPauseMenuProc(AMX* pAmx, cell* pParams)
{
	CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "IsPlayerInPauseMenu");
	return pPlayer ? pPlayer->IsInPauseMenu() : 0;
}

cell AMX_NATIVE_CALL GetPlayerResolutionProc(AMX* pAmx, cell* pParams)
{	
	CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "GetPlayerResolution");
	if (!pPlayer)
		return 0;

	cell *cellptr;
	amx_GetAddr(pAmx, pParams[2], &cellptr);
	*cellptr = pPlayer->GetResolutionX();

	amx_GetAddr(pAmx, pParams[3], &cellptr);
	*cellptr = pPlayer->GetResolutionY();

	return 1;
}

cell AMX_NATIVE_CALL SetWaveHeightForAllProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(amx_ctof(pParams[1])); // wave height

	Network::BroadcastRPC(eRPC::SET_WAVE_HEIGHT, &bitStream);
	return 1;
}

cell AMX_NATIVE_CALL SetPlayerCheckpointExProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write<float>(amx_ctof(pParams[2])); //pos x
	bitStream.Write<float>(amx_ctof(pParams[3])); //pos y
	bitStream.Write<float>(amx_ctof(pParams[4])); //pos z
	bitStream.Write<float>(amx_ctof(pParams[5])); //size
	bitStream.WriteCasted<unsigned int, cell>(pParams[6]); //colour
	bitStream.WriteCasted<unsigned short, cell>(pParams[7]); //period
	bitStream.Write<float>(amx_ctof(pParams[8])); //pulse
	bitStream.WriteCasted<short, cell>(pParams[9]); //rot_rate
	bitStream.Write(!!pParams[10]); //check_z

	DisablePlayerCheckpoint(pParams[1]);

	Network::PlayerSendRPC(eRPC::SET_CHECKPOINT_EX, pParams[1], &bitStream);
	return 1;
}

cell AMX_NATIVE_CALL SetPlayerRaceCheckpointExProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.WriteCasted<unsigned char, cell>(pParams[2]); //type
	bitStream.Write<float>(amx_ctof(pParams[3])); //pos x
	bitStream.Write<float>(amx_ctof(pParams[4])); //pos y
	bitStream.Write<float>(amx_ctof(pParams[5])); //pos z
	bitStream.Write<float>(amx_ctof(pParams[6])); //point x
	bitStream.Write<float>(amx_ctof(pParams[7])); //point y
	bitStream.Write<float>(amx_ctof(pParams[8])); //point z
	bitStream.Write<float>(amx_ctof(pParams[9])); //size
	bitStream.WriteCasted<unsigned int, cell>(pParams[10]); //colour
	bitStream.WriteCasted<unsigned short, cell>(pParams[11]); //period
	bitStream.Write<float>(amx_ctof(pParams[12])); //pulse
	bitStream.WriteCasted<short, cell>(pParams[13]); //rot_rate

	DisablePlayerRaceCheckpoint(pParams[1]);

	Network::PlayerSendRPC(eRPC::SET_RACE_CHECKPOINT_EX, pParams[1], &bitStream);
	return 1;
}

cell AMX_NATIVE_CALL SetPlayerCheckpointColourProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.WriteCasted<unsigned long, cell>(pParams[2]); // checkpoint colours
	/*bitStream.WriteCasted<unsigned long, cell>(pParams[3]);
	bitStream.WriteCasted<unsigned long, cell>(pParams[4]);*/

	Network::PlayerSendRPC(eRPC::SET_CHECKPOINT_COLOUR, pParams[1], &bitStream);
	return 1;
}

cell AMX_NATIVE_CALL SetPlayerRaceCheckpointColourProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.WriteCasted<unsigned int, cell>(pParams[2]); // checkpoint colour

	Network::PlayerSendRPC(eRPC::SET_RACE_CHECKPOINT_COLOUR, pParams[1], &bitStream);
	return 1;
}

cell AMX_NATIVE_CALL TogglePlayerActionProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.WriteCasted<unsigned char, cell>(pParams[2]); // action
	bitStream.Write(!!pParams[3]); // toggle

	return Network::PlayerSendRPC(eRPC::TOGGLE_ACTION, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL SetPlayerClipAmmoProc(AMX* pAmx, cell* pParams) // tips off SA-MP's anti-cheat, wouldn't recomend using... yet...
{
	RakNet::BitStream bitStream;
	bitStream.WriteCasted<unsigned char, cell>(pParams[2]); // bSlotId
	bitStream.WriteCasted<DWORD, cell>(pParams[3]); // dwNewAmmo

	return Network::PlayerSendRPC(eRPC::SET_CLIP_AMMO, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL SetPlayerNoReloadProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]); // bToggle
	
	return Network::PlayerSendRPC(eRPC::SET_NO_RELOAD, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL SetPlayerBlurIntensityProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(pParams[2]);

	return Network::PlayerSendRPC(eRPC::SET_BLUR_INTENSITY, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL TogglePlayerDriveOnWaterProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]);

	return Network::PlayerSendRPC(eRPC::TOGGLE_DRIVE_ON_WATER, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL SetPlayerGameSpeedProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(amx_ctof(pParams[2]));

	return Network::PlayerSendRPC(eRPC::SET_GAME_SPEED, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL TogglePlayerFrozenProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]);

	return Network::PlayerSendRPC(eRPC::TOGGLE_PLAYER_FROZEN, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL SetPedAnimsProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]);

	return Network::PlayerSendRPC(eRPC::SET_PLAYER_ANIMS, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL TogglePlayerSwitchReloadProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]);

	return Network::PlayerSendRPC(eRPC::TOGGLE_SWITCH_RELOAD, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL TogglePlayerInfiniteRunProc(AMX* pAmx, cell* pParams)
{
	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]);

	return Network::PlayerSendRPC(eRPC::TOGGLE_INFINITE_RUN, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL SetPlayerAircraftHeightProc(AMX* pAmx, cell* pParams)
{
	CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "SetPlayerAircraftHeight");
	if (!pPlayer)
		return 0;

	RakNet::BitStream bitStream;
	bitStream.Write(amx_ctof(pParams[2]));

	pPlayer->SetAircraftHeight(amx_ctof(pParams[2]));
	return Network::PlayerSendRPC(eRPC::SET_AIRCRAFT_HEIGHT, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL GetPlayerAircraftHeightProc(AMX* pAmx, cell* pParams)
{
	CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "GetPlayerAircraftHeight");
	if (!pPlayer)
	{
		float fallback = 0.0f;
		return amx_ftoc(fallback);
	}

	float f = pPlayer->GetAircraftHeight();
	return amx_ftoc(f);
}

cell AMX_NATIVE_CALL SetPlayerJetpackHeightProc(AMX* pAmx, cell* pParams)
{
	CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "SetPlayerJetpackHeight");
	if (!pPlayer)
		return 0;

	RakNet::BitStream bitStream;
	bitStream.Write(amx_ctof(pParams[2]));

	pPlayer->SetJetpackHeight(amx_ctof(pParams[2]));
	return Network::PlayerSendRPC(eRPC::SET_JETPACK_HEIGHT, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL GetPlayerJetpackHeightProc(AMX* pAmx, cell* pParams)
{
	CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "GetPlayerJetpackHeight");
	if (!pPlayer)
	{
		float fallback = 0.0f;
		return amx_ftoc(fallback);
	}

	float f = pPlayer->GetJetpackHeight();
	return amx_ftoc(f);
}

cell AMX_NATIVE_CALL TogglePlayerVehicleBlipsProc(AMX* pAmx, cell* pParams)
{
	CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "TogglePlayerVehicleBlips");
	if (!pPlayer)
		return 0;

	RakNet::BitStream bitStream;
	bitStream.Write(!!pParams[2]);

	pPlayer->ToggleVehicleBlips(!!pParams[2]);
	return Network::PlayerSendRPC(eRPC::TOGGLE_VEHICLE_BLIPS, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL GetPlayerVehicleBlipsProc(AMX* pAmx, cell* pParams)
{
	CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "GetPlayerVehicleBlips");
	return pPlayer ? pPlayer->GetVehicleBlips() : 0;
}

cell AMX_NATIVE_CALL IsUsingSAMPPProc(AMX* pAmx, cell* pParams)
{
	return Network::isConnected(pParams[1]);
}

cell AMX_NATIVE_CALL HasFeatureProc(AMX* pAmx, cell* pParams)
{
	if (!Network::isConnected(pParams[1]))
		return 0;

	const uint32_t features = OMPPlusProtocol::FeatureHUD | OMPPlusProtocol::FeatureKeybind | OMPPlusProtocol::FeatureKeyCapture;
	return (features & static_cast<uint32_t>(pParams[2])) != 0 ? 1 : 0;
}

cell AMX_NATIVE_CALL GetClientFeatureFlagsProc(AMX* pAmx, cell* pParams)
{
	if (!Network::isConnected(pParams[1]))
		return 0;

	return static_cast<cell>(OMPPlusProtocol::FeatureHUD | OMPPlusProtocol::FeatureKeybind | OMPPlusProtocol::FeatureKeyCapture);
}

cell AMX_NATIVE_CALL GetClientCapabilitiesProc(AMX* pAmx, cell* pParams)
{
	return Network::isConnected(pParams[1]) ? static_cast<cell>(OMPPlusProtocol::CapabilityKeyCapture) : 0;
}

cell AMX_NATIVE_CALL GetClientVersionProc(AMX* pAmx, cell* pParams)
{
	if (!Network::isConnected(pParams[1]))
		return 0;

	cell* pMajor = NULL;
	cell* pMinor = NULL;
	cell* pPatch = NULL;
	if (amx_GetAddr(pAmx, pParams[2], &pMajor) != AMX_ERR_NONE
		|| amx_GetAddr(pAmx, pParams[3], &pMinor) != AMX_ERR_NONE
		|| amx_GetAddr(pAmx, pParams[4], &pPatch) != AMX_ERR_NONE)
		return 0;

	*pMajor = 0;
	*pMinor = 0;
	*pPatch = 0;
	return 1;
}

cell AMX_NATIVE_CALL GetClientHashProc(AMX* pAmx, cell* pParams)
{
	if (!Network::isConnected(pParams[1]))
		return 0;

	cell* pDest = NULL;
	if (amx_GetAddr(pAmx, pParams[2], &pDest) != AMX_ERR_NONE || !pDest)
		return 0;

	const int size = pParams[0] >= 3 * sizeof(cell) ? static_cast<int>(pParams[3]) : 1;
	return amx_SetString(pDest, "", false, false, size) == AMX_ERR_NONE ? 1 : 0;
}

cell AMX_NATIVE_CALL IsLauncherVerifiedProc(AMX* pAmx, cell* pParams)
{
	return 0;
}

cell AMX_NATIVE_CALL BindKeyProc(AMX* pAmx, cell* pParams)
{
	CPlayer* pPlayer = GetSAMPPPlayer(pParams[1], "SAMPP_BindKey");
	if (!pPlayer)
		return 0;

	unsigned short key = (unsigned short)pParams[2];
	unsigned char eventMask = (unsigned char)(pParams[3] & 0x03);
	std::string action = pParams[0] >= 4 * sizeof(cell) ? GetPawnString(pAmx, pParams[4], 31) : "";

	if (!key || key > 255 || !eventMask)
		return 0;

	unsigned char actionLength = (unsigned char)action.length();
	RakNet::BitStream bitStream;
	bitStream.Write(key);
	bitStream.Write(eventMask);
	bitStream.Write(actionLength);

	if (actionLength)
		bitStream.Write(action.c_str(), actionLength);

	return Network::PlayerSendRPC(eRPC::SET_KEY_BIND, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL UnbindKeyProc(AMX* pAmx, cell* pParams)
{
	if (!GetSAMPPPlayer(pParams[1], "SAMPP_UnbindKey"))
		return 0;

	unsigned short key = (unsigned short)pParams[2];
	if (!key || key > 255)
		return 0;

	RakNet::BitStream bitStream;
	bitStream.Write(key);
	return Network::PlayerSendRPC(eRPC::UNBIND_KEY, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL ClearKeyBindsProc(AMX* pAmx, cell* pParams)
{
	if (!GetSAMPPPlayer(pParams[1], "SAMPP_ClearKeyBinds"))
		return 0;

	return Network::PlayerSendRPC(eRPC::CLEAR_KEY_BINDS, pParams[1]);
}

cell AMX_NATIVE_CALL BeginKeyCaptureProc(AMX* pAmx, cell* pParams)
{
	if (!GetSAMPPPlayer(pParams[1], "SAMPP_BeginKeyCapture"))
		return 0;

	unsigned short key = (unsigned short)pParams[2];
	unsigned char eventMask = (unsigned char)(pParams[3] & 0x03);
	int priorityRaw = (int)pParams[4];
	int ttlRaw = (int)pParams[5];
	unsigned char flags = (unsigned char)(pParams[6] & 0xFF);
	std::string action = pParams[0] >= 7 * sizeof(cell) ? GetPawnString(pAmx, pParams[7], 31) : "";

	if (!key || key > 255 || !eventMask || ttlRaw < 0)
		return 0;

	int priority = priorityRaw;
	if (priority < -32768)
		priority = -32768;
	else if (priority > 32767)
		priority = 32767;

	int ttl = ttlRaw;
	if (ttl < 0)
		ttl = 0;
	else if (ttl > 5000)
		ttl = 5000;
	unsigned char actionLength = (unsigned char)action.length();

	RakNet::BitStream bitStream;
	bitStream.Write(key);
	bitStream.Write(eventMask);
	bitStream.Write(flags);
	bitStream.Write((short)priority);
	bitStream.Write((unsigned short)ttl);
	bitStream.Write(actionLength);

	if (actionLength)
		bitStream.Write(action.c_str(), actionLength);

	return Network::PlayerSendRPC(eRPC::SET_KEY_CAPTURE, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL EndKeyCaptureProc(AMX* pAmx, cell* pParams)
{
	if (!GetSAMPPPlayer(pParams[1], "SAMPP_EndKeyCapture"))
		return 0;

	unsigned short key = (unsigned short)pParams[2];
	std::string action = pParams[0] >= 3 * sizeof(cell) ? GetPawnString(pAmx, pParams[3], 31) : "";
	if (!key || key > 255)
		return 0;

	unsigned char actionLength = (unsigned char)action.length();
	RakNet::BitStream bitStream;
	bitStream.Write(key);
	bitStream.Write(actionLength);

	if (actionLength)
		bitStream.Write(action.c_str(), actionLength);

	return Network::PlayerSendRPC(eRPC::CLEAR_KEY_CAPTURE, pParams[1], &bitStream);
}

cell AMX_NATIVE_CALL ClearKeyCapturesProc(AMX* pAmx, cell* pParams)
{
	if (!GetSAMPPPlayer(pParams[1], "SAMPP_ClearKeyCaptures"))
		return 0;

	return Network::PlayerSendRPC(eRPC::CLEAR_KEY_CAPTURES, pParams[1]);
}

PLUGIN_EXPORT unsigned int PLUGIN_CALL Supports()
{
	return SUPPORTS_VERSION | SUPPORTS_AMX_NATIVES | SUPPORTS_PROCESS_TICK;
}

PLUGIN_EXPORT void PLUGIN_CALL ProcessTick()
{
	Network::Process();
}

PLUGIN_EXPORT bool PLUGIN_CALL Load(void **ppData)
{
	Utility::Initialize(ppData);
	Utility::Printf("Loaded");
	SAMPServer::Initialize("config.json");
	Utility::Printf("Using server port %u, SA-MP+ port %u, max players %u",
		SAMPServer::GetListeningPort(),
		SAMPServer::GetListeningPort() + 1,
		SAMPServer::getMaxPlayers());
	Network::Initialize(SAMPServer::GetListeningAddress(), SAMPServer::GetListeningPort()+1, SAMPServer::getMaxPlayers());
	return sampgdk::Load(ppData);
}

PLUGIN_EXPORT void PLUGIN_CALL Unload()
{
	Utility::Printf("Unloaded");
	return sampgdk::Unload();
}

AMX_NATIVE_INFO PluginNatives[] =
{
	{ "SAMPP_ExecuteCallback", CallbackProc },
	{ "ToggleHUDComponentForPlayer", ToggleHUDComponentForPlayerProc },
	{ "SetRadioStationForPlayer", SetRadioStationForPlayerProc },
	{ "SetWaveHeightForPlayer", SetWaveHeightForPlayerProc },
	{ "SetWaveHeightForAll", SetWaveHeightForAllProc },
	{ "TogglePauseMenuAbility", TogglePauseMenuAbilityProc },
	{ "IsPlayerInPauseMenu", IsPlayerInPauseMenuProc },
	{ "SetPlayerHUDComponentColour", SetPlayerHUDComponentColourProc },
	{ "SetPlayerCheckpointEx", SetPlayerCheckpointExProc },
	{ "SetPlayerRaceCheckpointEx", SetPlayerRaceCheckpointExProc },
	{ "SetPlayerCheckpointColour", SetPlayerCheckpointColourProc },
	{ "SetPlayerRaceCheckpointColour", SetPlayerRaceCheckpointColourProc },
	{ "TogglePlayerAction", TogglePlayerActionProc },
	//{ "SetPlayerAmmoInClip", SetPlayerClipAmmoProc }, // tips off SA-MP's anti-cheat, wouldn't recomend using... yet...
	{ "SetPlayerNoReload", SetPlayerNoReloadProc },
	{ "GetPlayerResolution", GetPlayerResolutionProc },
	{ "IsUsingSAMPP", IsUsingSAMPPProc },
	{ "IsUsingOMPPlus", IsUsingSAMPPProc },
	{ "SAMPP_HasFeature", HasFeatureProc },
	{ "SAMPP_GetClientFeatureFlags", GetClientFeatureFlagsProc },
	{ "SAMPP_GetClientCapabilities", GetClientCapabilitiesProc },
	{ "SAMPP_GetClientVersion", GetClientVersionProc },
	{ "SAMPP_GetClientHash", GetClientHashProc },
	{ "SAMPP_IsLauncherVerified", IsLauncherVerifiedProc },
	{ "SetPlayerBlurIntensity", SetPlayerBlurIntensityProc },
	{ "TogglePlayerDriveOnWater", TogglePlayerDriveOnWaterProc },
	{ "SetPlayerGameSpeed", SetPlayerGameSpeedProc },
	{ "TogglePlayerFrozen", TogglePlayerFrozenProc },
	{ "SetPlayerPedAnims", SetPedAnimsProc },
	{ "TogglePlayerSwitchReload", TogglePlayerSwitchReloadProc },
	{ "TogglePlayerInfiniteRun", TogglePlayerInfiniteRunProc },
	{ "SetPlayerAircraftHeight", SetPlayerAircraftHeightProc },
	{ "GetPlayerAircraftHeight", GetPlayerAircraftHeightProc },
	{ "SetPlayerJetpackHeight", SetPlayerJetpackHeightProc },
	{ "GetPlayerJetpackHeight", GetPlayerJetpackHeightProc },
	{ "TogglePlayerVehicleBlips", TogglePlayerVehicleBlipsProc },
	{ "GetPlayerVehicleBlips", GetPlayerVehicleBlipsProc },
	{ "GetPlayerRadioStation", GetRadioStation },
	{ "TogglePlayerInfiniteOxygen", ToggleInfiniteOxygen },
	{ "ToggleWaterBuoyancy", TogglePlayerWaterBuoyancy },
	{ "ToggleUnderwaterEffect", ToggleUnderwaterEffect },
	{ "ToggleNightVision", ToggleNightVision },
	{ "ToggleThermalVision", ToggleThermalVision },
	{ "SAMPP_BindKey", BindKeyProc },
	{ "SAMPP_UnbindKey", UnbindKeyProc },
	{ "SAMPP_ClearKeyBinds", ClearKeyBindsProc },
	{ "SAMPP_BeginKeyCapture", BeginKeyCaptureProc },
	{ "SAMPP_EndKeyCapture", EndKeyCaptureProc },
	{ "SAMPP_ClearKeyCaptures", ClearKeyCapturesProc },
	{ 0, 0 }
};

PLUGIN_EXPORT int PLUGIN_CALL AmxLoad(AMX *pAmx)
{
	Callback::GetAMXList().push_back(pAmx);
	return amx_Register(pAmx, PluginNatives, -1);
}


PLUGIN_EXPORT int PLUGIN_CALL AmxUnload(AMX *pAmx)
{
	Callback::GetAMXList().remove(pAmx);
	return AMX_ERR_NONE;
}
