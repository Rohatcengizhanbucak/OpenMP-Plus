#include "omp_plus_component.hpp"

#include <algorithm>
#include <cstdint>

namespace
{
	using namespace OMPPlusProtocol;

	OMPPlusComponent* Component()
	{
		return OMPPlusComponent::getInstance();
	}

	int ParamCount(cell* params)
	{
		return static_cast<int>(params[0] / sizeof(cell));
	}

	template <typename T>
	void WriteCell(NetworkBitStream& stream, cell value)
	{
		stream.Write(static_cast<T>(value));
	}

	cell SendEmpty(cell playerid, uint16_t rpc)
	{
		return Component()->sendLegacyRPC(static_cast<int>(playerid), rpc) ? 1 : 0;
	}

	cell SendBool(cell playerid, uint16_t rpc, cell value)
	{
		NetworkBitStream stream;
		stream.Write(value != 0);
		return Component()->sendLegacyRPC(static_cast<int>(playerid), rpc, &stream) ? 1 : 0;
	}

	cell SendUInt8(cell playerid, uint16_t rpc, cell value)
	{
		NetworkBitStream stream;
		WriteCell<uint8_t>(stream, value);
		return Component()->sendLegacyRPC(static_cast<int>(playerid), rpc, &stream) ? 1 : 0;
	}

	cell SendFloat(cell playerid, uint16_t rpc, cell value)
	{
		NetworkBitStream stream;
		stream.Write(amx_ctof(value));
		return Component()->sendLegacyRPC(static_cast<int>(playerid), rpc, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL CallbackProc(AMX*, cell*)
	{
		// Native component mode gets player lifecycle directly from open.mp.
		return 1;
	}

	cell AMX_NATIVE_CALL ToggleHUDComponentForPlayerProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		WriteCell<uint8_t>(stream, params[2]);
		stream.Write(params[3] != 0);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), TOGGLE_HUD_COMPONENT, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL SetPlayerHUDComponentColourProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		WriteCell<uint8_t>(stream, params[2]);
		WriteCell<uint32_t>(stream, params[3]);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), SET_HUD_COMPONENT_COLOUR, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL SetRadioStationForPlayerProc(AMX*, cell* params)
	{
		return SendUInt8(params[1], SET_RADIO_STATION, params[2]);
	}

	cell AMX_NATIVE_CALL GetRadioStationProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		return state && state->ready ? state->radio : 0;
	}

	cell AMX_NATIVE_CALL SetWaveHeightForPlayerProc(AMX*, cell* params)
	{
		return SendFloat(params[1], SET_WAVE_HEIGHT, params[2]);
	}

	cell AMX_NATIVE_CALL SetWaveHeightForAllProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		stream.Write(amx_ctof(params[1]));
		Component()->broadcastLegacyRPC(SET_WAVE_HEIGHT, &stream);
		return 1;
	}

	cell AMX_NATIVE_CALL TogglePauseMenuAbilityProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_PAUSE_MENU, params[2]);
	}

	cell AMX_NATIVE_CALL ToggleInfiniteOxygenProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_INFINITE_OXYGEN, params[2]);
	}

	cell AMX_NATIVE_CALL TogglePlayerWaterBuoyancyProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_WATER_BUOYANCY, params[2]);
	}

	cell AMX_NATIVE_CALL ToggleUnderwaterEffectProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_UNDERWATER_EFFECT, params[2]);
	}

	cell AMX_NATIVE_CALL ToggleNightVisionProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_NIGHTVISION, params[2]);
	}

	cell AMX_NATIVE_CALL ToggleThermalVisionProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_THERMALVISION, params[2]);
	}

	cell AMX_NATIVE_CALL IsPlayerInPauseMenuProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		return state && state->ready && state->inPauseMenu ? 1 : 0;
	}

	cell AMX_NATIVE_CALL GetPlayerResolutionProc(AMX* amx, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		if (!state || !state->ready)
			return 0;

		Component()->setPawnCell(amx, params[2], static_cast<cell>(state->resolutionX));
		Component()->setPawnCell(amx, params[3], static_cast<cell>(state->resolutionY));
		return 1;
	}

	cell AMX_NATIVE_CALL SetPlayerCheckpointExProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		stream.Write(amx_ctof(params[2]));
		stream.Write(amx_ctof(params[3]));
		stream.Write(amx_ctof(params[4]));
		stream.Write(amx_ctof(params[5]));
		WriteCell<uint32_t>(stream, params[6]);
		WriteCell<uint16_t>(stream, params[7]);
		stream.Write(amx_ctof(params[8]));
		WriteCell<int16_t>(stream, params[9]);
		stream.Write(params[10] != 0);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), SET_CHECKPOINT_EX, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL SetPlayerRaceCheckpointExProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		WriteCell<uint8_t>(stream, params[2]);
		stream.Write(amx_ctof(params[3]));
		stream.Write(amx_ctof(params[4]));
		stream.Write(amx_ctof(params[5]));
		stream.Write(amx_ctof(params[6]));
		stream.Write(amx_ctof(params[7]));
		stream.Write(amx_ctof(params[8]));
		stream.Write(amx_ctof(params[9]));
		WriteCell<uint32_t>(stream, params[10]);
		WriteCell<uint16_t>(stream, params[11]);
		stream.Write(amx_ctof(params[12]));
		WriteCell<int16_t>(stream, params[13]);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), SET_RACE_CHECKPOINT_EX, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL SetPlayerCheckpointColourProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		WriteCell<uint32_t>(stream, params[2]);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), SET_CHECKPOINT_COLOUR, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL SetPlayerRaceCheckpointColourProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		WriteCell<uint32_t>(stream, params[2]);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), SET_RACE_CHECKPOINT_COLOUR, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL TogglePlayerActionProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		WriteCell<uint8_t>(stream, params[2]);
		stream.Write(params[3] != 0);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), TOGGLE_ACTION, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL SetPlayerClipAmmoProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		WriteCell<uint8_t>(stream, params[2]);
		WriteCell<uint32_t>(stream, params[3]);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), SET_CLIP_AMMO, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL SetPlayerNoReloadProc(AMX*, cell* params)
	{
		return SendBool(params[1], SET_NO_RELOAD, params[2]);
	}

	cell AMX_NATIVE_CALL SetPlayerBlurIntensityProc(AMX*, cell* params)
	{
		NetworkBitStream stream;
		WriteCell<int32_t>(stream, params[2]);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), SET_BLUR_INTENSITY, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL TogglePlayerDriveOnWaterProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_DRIVE_ON_WATER, params[2]);
	}

	cell AMX_NATIVE_CALL SetPlayerGameSpeedProc(AMX*, cell* params)
	{
		return SendFloat(params[1], SET_GAME_SPEED, params[2]);
	}

	cell AMX_NATIVE_CALL TogglePlayerFrozenProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_PLAYER_FROZEN, params[2]);
	}

	cell AMX_NATIVE_CALL SetPedAnimsProc(AMX*, cell* params)
	{
		return SendBool(params[1], SET_PLAYER_ANIMS, params[2]);
	}

	cell AMX_NATIVE_CALL TogglePlayerSwitchReloadProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_SWITCH_RELOAD, params[2]);
	}

	cell AMX_NATIVE_CALL TogglePlayerInfiniteRunProc(AMX*, cell* params)
	{
		return SendBool(params[1], TOGGLE_INFINITE_RUN, params[2]);
	}

	cell AMX_NATIVE_CALL SetPlayerAircraftHeightProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		if (state)
			state->aircraftHeight = amx_ctof(params[2]);
		return SendFloat(params[1], SET_AIRCRAFT_HEIGHT, params[2]);
	}

	cell AMX_NATIVE_CALL GetPlayerAircraftHeightProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		float value = state && state->ready ? state->aircraftHeight : 0.0f;
		return amx_ftoc(value);
	}

	cell AMX_NATIVE_CALL SetPlayerJetpackHeightProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		if (state)
			state->jetpackHeight = amx_ctof(params[2]);
		return SendFloat(params[1], SET_JETPACK_HEIGHT, params[2]);
	}

	cell AMX_NATIVE_CALL GetPlayerJetpackHeightProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		float value = state && state->ready ? state->jetpackHeight : 0.0f;
		return amx_ftoc(value);
	}

	cell AMX_NATIVE_CALL TogglePlayerVehicleBlipsProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		if (state)
			state->vehicleBlips = params[2] != 0;
		return SendBool(params[1], TOGGLE_VEHICLE_BLIPS, params[2]);
	}

	cell AMX_NATIVE_CALL GetPlayerVehicleBlipsProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		return state && state->ready && state->vehicleBlips ? 1 : 0;
	}

	cell AMX_NATIVE_CALL IsUsingSAMPPProc(AMX*, cell* params)
	{
		return Component()->isUsingOMPPlus(static_cast<int>(params[1])) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL IsUsingOMPPlusProc(AMX*, cell* params)
	{
		return Component()->isUsingOMPPlus(static_cast<int>(params[1])) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL HasFeatureProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		if (!state || !state->ready)
			return 0;

		const uint32_t feature = static_cast<uint32_t>(params[2]);
		return (state->featureFlags & feature) != 0 ? 1 : 0;
	}

	cell AMX_NATIVE_CALL GetClientFeatureFlagsProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		return state && state->ready ? static_cast<cell>(state->featureFlags) : 0;
	}

	cell AMX_NATIVE_CALL GetClientCapabilitiesProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		return state && state->ready ? static_cast<cell>(state->capabilities) : 0;
	}

	cell AMX_NATIVE_CALL GetClientVersionProc(AMX* amx, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		if (!state || !state->ready)
			return 0;

		Component()->setPawnCell(amx, params[2], static_cast<cell>(state->clientVersionMajor));
		Component()->setPawnCell(amx, params[3], static_cast<cell>(state->clientVersionMinor));
		Component()->setPawnCell(amx, params[4], static_cast<cell>(state->clientVersionPatch));
		return 1;
	}

	cell AMX_NATIVE_CALL GetClientHashProc(AMX* amx, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		if (!state || !state->ready)
			return 0;

		const cell size = ParamCount(params) >= 3 ? params[3] : static_cast<cell>(OMPPlusProtocol::MaxClientHashLength + 1);
		return Component()->setPawnString(amx, params[2], size, state->clientHash) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL IsLauncherVerifiedProc(AMX*, cell* params)
	{
		OMPPlusPlayerState* state = Component()->getPlayerState(static_cast<int>(params[1]));
		return state && state->ready && state->launcherVerified ? 1 : 0;
	}

	cell AMX_NATIVE_CALL BindKeyProc(AMX* amx, cell* params)
	{
		if (!Component()->isUsingOMPPlus(static_cast<int>(params[1])))
			return 0;

		const uint16_t key = static_cast<uint16_t>(params[2]);
		const uint8_t eventMask = static_cast<uint8_t>(params[3] & 0x03);
		const std::string action = ParamCount(params) >= 4 ? Component()->getPawnString(amx, params[4], 31) : std::string();

		if (!key || key > 255 || !eventMask)
			return 0;

		NetworkBitStream stream;
		stream.Write(key);
		stream.Write(eventMask);
		stream.Write(static_cast<uint8_t>(action.length()));
		if (!action.empty())
			stream.Write(action.c_str(), static_cast<int>(action.length()));

		return Component()->sendLegacyRPC(static_cast<int>(params[1]), SET_KEY_BIND, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL UnbindKeyProc(AMX*, cell* params)
	{
		if (!Component()->isUsingOMPPlus(static_cast<int>(params[1])))
			return 0;

		const uint16_t key = static_cast<uint16_t>(params[2]);
		if (!key || key > 255)
			return 0;

		NetworkBitStream stream;
		stream.Write(key);
		return Component()->sendLegacyRPC(static_cast<int>(params[1]), UNBIND_KEY, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL ClearKeyBindsProc(AMX*, cell* params)
	{
		return SendEmpty(params[1], CLEAR_KEY_BINDS);
	}

	cell AMX_NATIVE_CALL BeginKeyCaptureProc(AMX* amx, cell* params)
	{
		if (!Component()->isUsingOMPPlus(static_cast<int>(params[1])))
			return 0;

		const uint16_t key = static_cast<uint16_t>(params[2]);
		const uint8_t eventMask = static_cast<uint8_t>(params[3] & 0x03);
		const int priorityRaw = static_cast<int>(params[4]);
		const int ttlRaw = static_cast<int>(params[5]);
		const uint8_t flags = static_cast<uint8_t>(params[6] & 0xFF);
		const std::string action = ParamCount(params) >= 7 ? Component()->getPawnString(amx, params[7], 31) : std::string();

		if (!key || key > 255 || !eventMask || ttlRaw < 0)
			return 0;

		const int priority = std::max(-32768, std::min(32767, priorityRaw));
		const int ttl = std::max(0, std::min(5000, ttlRaw));

		NetworkBitStream stream;
		stream.Write(key);
		stream.Write(eventMask);
		stream.Write(flags);
		stream.Write(static_cast<int16_t>(priority));
		stream.Write(static_cast<uint16_t>(ttl));
		stream.Write(static_cast<uint8_t>(action.length()));
		if (!action.empty())
			stream.Write(action.c_str(), static_cast<int>(action.length()));

		return Component()->sendLegacyRPC(static_cast<int>(params[1]), SET_KEY_CAPTURE, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL EndKeyCaptureProc(AMX* amx, cell* params)
	{
		if (!Component()->isUsingOMPPlus(static_cast<int>(params[1])))
			return 0;

		const uint16_t key = static_cast<uint16_t>(params[2]);
		const std::string action = ParamCount(params) >= 3 ? Component()->getPawnString(amx, params[3], 31) : std::string();
		if (!key || key > 255)
			return 0;

		NetworkBitStream stream;
		stream.Write(key);
		stream.Write(static_cast<uint8_t>(action.length()));
		if (!action.empty())
			stream.Write(action.c_str(), static_cast<int>(action.length()));

		return Component()->sendLegacyRPC(static_cast<int>(params[1]), CLEAR_KEY_CAPTURE, &stream) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL ClearKeyCapturesProc(AMX*, cell* params)
	{
		return SendEmpty(params[1], CLEAR_KEY_CAPTURES);
	}

	cell AMX_NATIVE_CALL TargetBeginProc(AMX* amx, cell* params)
	{
		const int playerid = static_cast<int>(params[1]);
		const uint32_t targetid = static_cast<uint32_t>(params[2]);
		const std::string title = Component()->getPawnString(amx, params[3], 48);
		const uint16_t ttlMs = static_cast<uint16_t>(std::max<cell>(0, std::min<cell>(5000, params[4])));
		const uint32_t flags = ParamCount(params) >= 5 ? static_cast<uint32_t>(params[5]) : 0;
		return Component()->beginTargetContext(playerid, targetid, title, ttlMs, flags) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL TargetBeginExProc(AMX* amx, cell* params)
	{
		const int playerid = static_cast<int>(params[1]);
		const uint32_t targetid = static_cast<uint32_t>(params[2]);
		const uint8_t targetType = static_cast<uint8_t>(std::max<cell>(0, std::min<cell>(255, params[3])));
		const std::string title = Component()->getPawnString(amx, params[4], 48);
		const uint16_t ttlMs = static_cast<uint16_t>(std::max<cell>(0, std::min<cell>(5000, params[5])));
		const uint32_t flags = ParamCount(params) >= 6 ? static_cast<uint32_t>(params[6]) : 0;
		return Component()->beginTargetContextEx(playerid, targetid, targetType, title, ttlMs, flags) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL TargetSetLayoutProc(AMX*, cell* params)
	{
		const int playerid = static_cast<int>(params[1]);
		const uint32_t targetid = static_cast<uint32_t>(params[2]);
		const uint8_t layout = static_cast<uint8_t>(std::max<cell>(0, std::min<cell>(255, params[3])));
		return Component()->setTargetLayout(playerid, targetid, layout) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL TargetSetDescriptionProc(AMX* amx, cell* params)
	{
		const int playerid = static_cast<int>(params[1]);
		const uint32_t targetid = static_cast<uint32_t>(params[2]);
		const std::string description = Component()->getPawnString(amx, params[3], 128);
		return Component()->setTargetDescription(playerid, targetid, description) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL TargetAddOptionProc(AMX* amx, cell* params)
	{
		const int playerid = static_cast<int>(params[1]);
		const uint32_t targetid = static_cast<uint32_t>(params[2]);
		const uint32_t optionid = static_cast<uint32_t>(params[3]);
		const std::string label = Component()->getPawnString(amx, params[4], 48);
		const std::string icon = ParamCount(params) >= 5 ? Component()->getPawnString(amx, params[5], 24) : std::string();
		const bool enabled = ParamCount(params) >= 6 ? params[6] != 0 : true;
		return Component()->addTargetOption(playerid, targetid, optionid, label, icon, enabled) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL TargetAddRowProc(AMX* amx, cell* params)
	{
		const int playerid = static_cast<int>(params[1]);
		const uint32_t targetid = static_cast<uint32_t>(params[2]);
		const uint32_t optionid = static_cast<uint32_t>(params[3]);
		const uint8_t rowType = static_cast<uint8_t>(std::max<cell>(0, std::min<cell>(255, params[4])));
		const std::string label = Component()->getPawnString(amx, params[5], 96);
		const std::string icon = ParamCount(params) >= 6 ? Component()->getPawnString(amx, params[6], 24) : std::string();
		const bool enabled = ParamCount(params) >= 7 ? params[7] != 0 : true;
		return Component()->addTargetRow(playerid, targetid, optionid, rowType, label, icon, enabled) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL TargetCommitProc(AMX*, cell* params)
	{
		return Component()->commitTargetContext(static_cast<int>(params[1]), static_cast<uint32_t>(params[2])) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL TargetClearProc(AMX*, cell* params)
	{
		return Component()->clearTargetContext(static_cast<int>(params[1])) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL BuildOpenProc(AMX* amx, cell* params)
	{
		const int playerid = static_cast<int>(params[1]);
		const uint32_t sessionid = static_cast<uint32_t>(params[2]);
		const std::string title = Component()->getPawnString(amx, params[3], 48);
		const float maxDistance = ParamCount(params) >= 4 ? amx_ctof(params[4]) : 8.0f;
		return Component()->openBuild(playerid, sessionid, title, maxDistance) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL BuildCloseProc(AMX*, cell* params)
	{
		return Component()->closeBuild(static_cast<int>(params[1])) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL BuildClearPartsProc(AMX*, cell* params)
	{
		return Component()->clearBuildParts(static_cast<int>(params[1])) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL BuildAddPartProc(AMX* amx, cell* params)
	{
		const int playerid = static_cast<int>(params[1]);
		const uint32_t partid = static_cast<uint32_t>(params[2]);
		const int32_t modelid = static_cast<int32_t>(params[3]);
		const std::string name = Component()->getPawnString(amx, params[4], 48);
		const std::string category = ParamCount(params) >= 5 ? Component()->getPawnString(amx, params[5], 32) : std::string();
		const std::string cost = ParamCount(params) >= 6 ? Component()->getPawnString(amx, params[6], 48) : std::string();
		return Component()->addBuildPart(playerid, partid, modelid, name, category, cost) ? 1 : 0;
	}

	cell AMX_NATIVE_CALL BuildSendResultProc(AMX* amx, cell* params)
	{
		const int playerid = static_cast<int>(params[1]);
		const uint8_t result = static_cast<uint8_t>(std::max<cell>(0, std::min<cell>(255, params[2])));
		const std::string message = Component()->getPawnString(amx, params[3], 96);
		return Component()->sendBuildResult(playerid, result, message) ? 1 : 0;
	}
}

AMX_NATIVE_INFO OMPPlusNatives[] =
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
	{ "SetPlayerAmmoInClip", SetPlayerClipAmmoProc },
	{ "SetPlayerNoReload", SetPlayerNoReloadProc },
	{ "GetPlayerResolution", GetPlayerResolutionProc },
	{ "IsUsingSAMPP", IsUsingSAMPPProc },
	{ "IsUsingOMPPlus", IsUsingOMPPlusProc },
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
	{ "GetPlayerRadioStation", GetRadioStationProc },
	{ "TogglePlayerInfiniteOxygen", ToggleInfiniteOxygenProc },
	{ "ToggleWaterBuoyancy", TogglePlayerWaterBuoyancyProc },
	{ "ToggleUnderwaterEffect", ToggleUnderwaterEffectProc },
	{ "ToggleNightVision", ToggleNightVisionProc },
	{ "ToggleThermalVision", ToggleThermalVisionProc },
	{ "SAMPP_BindKey", BindKeyProc },
	{ "SAMPP_UnbindKey", UnbindKeyProc },
	{ "SAMPP_ClearKeyBinds", ClearKeyBindsProc },
	{ "SAMPP_BeginKeyCapture", BeginKeyCaptureProc },
	{ "SAMPP_EndKeyCapture", EndKeyCaptureProc },
	{ "SAMPP_ClearKeyCaptures", ClearKeyCapturesProc },
	{ "SAMPP_TargetBegin", TargetBeginProc },
	{ "SAMPP_TargetBeginEx", TargetBeginExProc },
	{ "SAMPP_TargetSetLayout", TargetSetLayoutProc },
	{ "SAMPP_TargetSetDescription", TargetSetDescriptionProc },
	{ "SAMPP_TargetAddOption", TargetAddOptionProc },
	{ "SAMPP_TargetAddRow", TargetAddRowProc },
	{ "SAMPP_TargetCommit", TargetCommitProc },
	{ "SAMPP_TargetClear", TargetClearProc },
	{ "SAMPP_BuildOpen", BuildOpenProc },
	{ "SAMPP_BuildClose", BuildCloseProc },
	{ "SAMPP_BuildClearParts", BuildClearPartsProc },
	{ "SAMPP_BuildAddPart", BuildAddPartProc },
	{ "SAMPP_BuildSendResult", BuildSendResultProc },
	{ nullptr, nullptr }
};
