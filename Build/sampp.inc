
#define HUD_COMPONENT_ALL 0
#define HUD_COMPONENT_AMMO 1
#define HUD_COMPONENT_WEAPON 2
#define HUD_COMPONENT_HEALTH 3
#define HUD_COMPONENT_BREATH 4
#define HUD_COMPONENT_ARMOUR 5
#define HUD_COMPONENT_MINIMAP 6
#define HUD_COMPONENT_CROSSHAIR 7
#define HUD_COMPONENT_MONEY 8

#define HUD_COLOUR_MONEY_POSITIVE 0
#define HUD_COLOUR_MONEY_NEGATIVE 1
#define HUD_COLOUR_ARMOUR 2
#define HUD_COLOUR_HEALTH 3
#define HUD_COLOUR_BREATH 4
#define HUD_COLOUR_AMMO 5
#define HUD_COLOUR_WANTED_LEVEL 6
#define HUD_COLOUR_RADIO_TUNED 7
#define HUD_COLOUR_RADIO_UNTUNED 8

#define RADIO_PLAYBACKFM 1
#define RADIO_KROSE 2
#define RADIO_KDST 3
#define RADIO_BOUNCEFM 4
#define RADIO_SFUR 5
#define RADIO_LOSSANTOS 6
#define RADIO_RADIOX 7
#define RADIO_CSR 8
#define RADIO_KJAHWEST 9
#define RADIO_MASTERSOUNDS 10
#define RADIO_WCTR 11
#define RADIO_UTP 12
#define RADIO_OFF 13

#define PAUSE_ID_STATS 0
#define PAUSE_ID_STARTGAME 1
#define PAUSE_ID_BRIEF 2
#define PAUSE_ID_AUDIOSETTINGS 3
#define PAUSE_ID_DISPLAYSETTINGS 4
#define PAUSE_ID_MAP 5
#define PAUSE_ID_DEFAULTSETTINGS 23
#define PAUSE_ID_AUDIODEFAULTSETTINGS 24
#define PAUSE_ID_CONTROLLERDEFAULTSETTINGS 25
#define PAUSE_ID_USERTRACKOPTIONS 26
#define PAUSE_ID_LANGUAGE 28
#define PAUSE_ID_OPTIONS 33
#define PAUSE_ID_QUITGAME 35
#define PAUSE_ID_CONTROLLERSETUP 36
#define PAUSE_ID_REDEFINECONTROLS 37
#define PAUSE_ID_FOOTVEHICLECONTROLS 38
#define PAUSE_ID_MOUSESETTINGS 39
#define PAUSE_ID_JOYPADSETTINGS 40
#define PAUSE_ID_MAIN 41

#define PLAYER_ACTION_ALL 0
#define PLAYER_ACTION_SPRINT 1
#define PLAYER_ACTION_ENTER_CAR 2
#define PLAYER_ACTION_CROUCH 3
#define PLAYER_ACTION_FIRE_WEAPON 4
#define PLAYER_ACTION_UNKNOWN 5
#define PLAYER_ACTION_SWITCH_WEAPON 6
#define PLAYER_ACTION_JUMP 7

#define STUNT_TYPE_TWO_WHEELS 247
#define STUNT_TYPE_INSANE 173
#define STUNT_TYPE_WHEELIE 117
#define STUNT_TYPE_STOPPIE 233

#define DEFAULT_BLUR_INTENSITY 36
#define NO_BLUR_INTENSITY 0

#define DEFAULT_AIRCRAFT_HEIGHT 800
#define DEFAULT_JETPACK_HEIGHT 100

#define MOUSE_LEFT_CLICK 0
#define MOUSE_RIGHT_CLICK 1
#define MOUSE_MIDDLE_CLICK 2

#define SAMPP_KEY_STATE_UP 0
#define SAMPP_KEY_STATE_DOWN 1

#define SAMPP_KEY_EVENT_DOWN 1
#define SAMPP_KEY_EVENT_UP 2
#define SAMPP_KEY_EVENT_BOTH 3

#define SAMPP_FEATURE_HUD 1
#define SAMPP_FEATURE_KEYBIND 2
#define SAMPP_FEATURE_KEYCAPTURE 4
#define SAMPP_FEATURE_AUDIO 8
#define SAMPP_FEATURE_EFFECTS 16
#define SAMPP_FEATURE_UI 32
#define SAMPP_FEATURE_TARGET 64
#define SAMPP_FEATURE_BUILD 128

#define SAMPP_CAPABILITY_NATIVE_TRANSPORT 1
#define SAMPP_CAPABILITY_KEY_CAPTURE 2
#define SAMPP_CAPABILITY_TARGET_UI 4
#define SAMPP_CAPABILITY_TARGET_UI_V2 8
#define SAMPP_CAPABILITY_BUILD_UI 16

#define SAMPP_CAPTURE_CONSUME_GAME_INPUT 1
#define SAMPP_CAPTURE_BLOCK_SWITCH_WEAPON 2
#define SAMPP_CAPTURE_DEFAULT_FLAGS (SAMPP_CAPTURE_CONSUME_GAME_INPUT | SAMPP_CAPTURE_BLOCK_SWITCH_WEAPON)
#define SAMPP_CAPTURE_LEASE_DEFAULT_MS 400
#define SAMPP_CAPTURE_REFRESH_DEFAULT_MS 150
#define SAMPP_CAPTURE_PRIORITY_LOW 0
#define SAMPP_CAPTURE_PRIORITY_ITEM 20
#define SAMPP_CAPTURE_PRIORITY_VEHICLE 40
#define SAMPP_CAPTURE_PRIORITY_MENU 60

#define SAMPP_TARGET_FLAG_HIDE_PROMPT 1
#define SAMPP_TARGET_FLAG_DIRECT_SELECT 2
#define SAMPP_TARGET_FLAG_PAYLOAD_V2 1073741824

#define SAMPP_TARGET_TYPE_GENERIC 0
#define SAMPP_TARGET_TYPE_VEHICLE 1
#define SAMPP_TARGET_TYPE_HOUSE 2
#define SAMPP_TARGET_TYPE_NPC 3
#define SAMPP_TARGET_TYPE_ACTOR 4
#define SAMPP_TARGET_TYPE_OBJECT 5
#define SAMPP_TARGET_TYPE_ITEM 6
#define SAMPP_TARGET_TYPE_PLAYER 7
#define SAMPP_TARGET_TYPE_CUSTOM 8

#define SAMPP_TARGET_LAYOUT_AUTO 0
#define SAMPP_TARGET_LAYOUT_COMPACT 1
#define SAMPP_TARGET_LAYOUT_STANDARD 2
#define SAMPP_TARGET_LAYOUT_DIALOG 3
#define SAMPP_TARGET_LAYOUT_WIDE 4
#define SAMPP_TARGET_LAYOUT_MINIMAL 5
#define SAMPP_TARGET_LAYOUT_CATEGORY 6

#define SAMPP_TARGET_ROW_ACTION 0
#define SAMPP_TARGET_ROW_INFO 1
#define SAMPP_TARGET_ROW_DIALOG 2
#define SAMPP_TARGET_ROW_DIVIDER 3
#define SAMPP_TARGET_ROW_HEADER 4
#define SAMPP_TARGET_ROW_DISABLED 5
#define SAMPP_TARGET_ROW_TOGGLE 6
#define SAMPP_TARGET_ROW_DANGER 7

#define SAMPP_BUILD_RESULT_SUCCESS 1
#define SAMPP_BUILD_RESULT_ERROR 2
#define SAMPP_BUILD_RESULT_PREVIEW_VALID 3
#define SAMPP_BUILD_RESULT_PREVIEW_INVALID 4

#define SAMPP_BUILD_PART_NONE 0
#define SAMPP_BUILD_PART_FOUNDATION 1
#define SAMPP_BUILD_PART_WALL 2
#define SAMPP_BUILD_PART_DOORFRAME 3
#define SAMPP_BUILD_PART_FLOOR 4
#define SAMPP_BUILD_PART_ROOF 5
#define SAMPP_BUILD_PART_STAIRS 6
#define SAMPP_BUILD_PART_DOOR 7
#define SAMPP_BUILD_PART_REMOVE 100
#define SAMPP_BUILD_ROTATION_STEP_DEGREES 90

#define SAMPP_KEY_LBUTTON 0x01
#define SAMPP_KEY_RBUTTON 0x02
#define SAMPP_KEY_MBUTTON 0x04
#define SAMPP_KEY_XBUTTON1 0x05
#define SAMPP_KEY_XBUTTON2 0x06
#define SAMPP_KEY_BACKSPACE 0x08
#define SAMPP_KEY_TAB 0x09
#define SAMPP_KEY_ENTER 0x0D
#define SAMPP_KEY_SHIFT 0x10
#define SAMPP_KEY_CONTROL 0x11
#define SAMPP_KEY_ALT 0x12
#define SAMPP_KEY_PAUSE 0x13
#define SAMPP_KEY_CAPSLOCK 0x14
#define SAMPP_KEY_ESCAPE 0x1B
#define SAMPP_KEY_SPACE 0x20
#define SAMPP_KEY_PAGEUP 0x21
#define SAMPP_KEY_PAGEDOWN 0x22
#define SAMPP_KEY_END 0x23
#define SAMPP_KEY_HOME 0x24
#define SAMPP_KEY_LEFT 0x25
#define SAMPP_KEY_UP 0x26
#define SAMPP_KEY_RIGHT 0x27
#define SAMPP_KEY_DOWN 0x28
#define SAMPP_KEY_INSERT 0x2D
#define SAMPP_KEY_DELETE 0x2E
#define SAMPP_KEY_0 0x30
#define SAMPP_KEY_1 0x31
#define SAMPP_KEY_2 0x32
#define SAMPP_KEY_3 0x33
#define SAMPP_KEY_4 0x34
#define SAMPP_KEY_5 0x35
#define SAMPP_KEY_6 0x36
#define SAMPP_KEY_7 0x37
#define SAMPP_KEY_8 0x38
#define SAMPP_KEY_9 0x39
#define SAMPP_KEY_A 0x41
#define SAMPP_KEY_B 0x42
#define SAMPP_KEY_C 0x43
#define SAMPP_KEY_D 0x44
#define SAMPP_KEY_E 0x45
#define SAMPP_KEY_F 0x46
#define SAMPP_KEY_G 0x47
#define SAMPP_KEY_H 0x48
#define SAMPP_KEY_I 0x49
#define SAMPP_KEY_J 0x4A
#define SAMPP_KEY_K 0x4B
#define SAMPP_KEY_L 0x4C
#define SAMPP_KEY_M 0x4D
#define SAMPP_KEY_N 0x4E
#define SAMPP_KEY_O 0x4F
#define SAMPP_KEY_P 0x50
#define SAMPP_KEY_Q 0x51
#define SAMPP_KEY_R 0x52
#define SAMPP_KEY_S 0x53
#define SAMPP_KEY_T 0x54
#define SAMPP_KEY_U 0x55
#define SAMPP_KEY_V 0x56
#define SAMPP_KEY_W 0x57
#define SAMPP_KEY_X 0x58
#define SAMPP_KEY_Y 0x59
#define SAMPP_KEY_Z 0x5A
#define SAMPP_KEY_NUMPAD0 0x60
#define SAMPP_KEY_NUMPAD1 0x61
#define SAMPP_KEY_NUMPAD2 0x62
#define SAMPP_KEY_NUMPAD3 0x63
#define SAMPP_KEY_NUMPAD4 0x64
#define SAMPP_KEY_NUMPAD5 0x65
#define SAMPP_KEY_NUMPAD6 0x66
#define SAMPP_KEY_NUMPAD7 0x67
#define SAMPP_KEY_NUMPAD8 0x68
#define SAMPP_KEY_NUMPAD9 0x69
#define SAMPP_KEY_F1 0x70
#define SAMPP_KEY_F2 0x71
#define SAMPP_KEY_F3 0x72
#define SAMPP_KEY_F4 0x73
#define SAMPP_KEY_F5 0x74
#define SAMPP_KEY_F6 0x75
#define SAMPP_KEY_F7 0x76
#define SAMPP_KEY_F8 0x77
#define SAMPP_KEY_F9 0x78
#define SAMPP_KEY_F10 0x79
#define SAMPP_KEY_F11 0x7A
#define SAMPP_KEY_F12 0x7B

native SetPlayerCheckpointEx(playerid, Float:x, Float:y, Float:z, Float:size, colour = 0xFF000020, period = 1024, Float:pulse = 0.1, rotation_rate = 0, bool:check_z = true);
native SetPlayerCheckpointColour(playerid, colour);

native SetPlayerRaceCheckpointEx(playerid, type, Float:x, Float:y, Float:z, Float:point_x, Float:point_y, Float:point_z, Float:size, colour = 0xFF000020, period = 1024, Float:pulse = 0.1, rotation_rate = 0);
native SetPlayerRaceCheckpointColour(playerid, colour);

native ToggleHUDComponentForPlayer(playerid, componentid, bool:toggle);
native SetRadioStationForPlayer(playerid, stationid);
native SetWaveHeightForPlayer(playerid, Float:height);
native SetWaveHeightForAll(Float:height);
native TogglePauseMenuAbility(playerid, bool:toggle);
native IsPlayerInPauseMenu(playerid);
native SetPlayerHUDComponentColour(playerid, componentid, colour);
native TogglePlayerAction(playerid, actionid, bool:toggle);
native SetPlayerBlurIntensity(playerid, intensity);
native SetPlayerGameSpeed(playerid, Float:speed);
native TogglePlayerDriveOnWater(playerid, bool:toggle);
native TogglePlayerFrozen(playerid, bool:toggle);
native SetPlayerPedAnims(playerid, bool:toggle);
native TogglePlayerSwitchReload(playerid, bool:toggle);
native bool:IsUsingSAMPP(playerid);
native bool:IsUsingOMPPlus(playerid);
native bool:SAMPP_HasFeature(playerid, feature);
native SAMPP_GetClientFeatureFlags(playerid);
native SAMPP_GetClientCapabilities(playerid);
native SAMPP_GetClientVersion(playerid, &major, &minor, &patch);
native SAMPP_GetClientHash(playerid, dest[], size = sizeof dest);
native bool:SAMPP_IsLauncherVerified(playerid);
native GetPlayerResolution(playerid, &X, &Y);
native SetPlayerNoReload(playerid, bool:toggle);
native TogglePlayerInfiniteRun(playerid, bool:toggle);
native SetPlayerAircraftHeight(playerid, Float:height);
native Float:GetPlayerAircraftHeight(playerid);
native SetPlayerJetpackHeight(playerid, Float:height);
native Float:GetPlayerJetpackHeight(playerid);
native TogglePlayerVehicleBlips(playerid, bool:toggle);
native GetPlayerVehicleBlips(playerid);
native GetPlayerRadioStation(playerid);
native TogglePlayerInfiniteOxygen(playerid, bool:toggle);
native ToggleWaterBuoyancy(playerid, bool:toggle);
native ToggleUnderwaterEffect(playerid, bool:toggle);
native ToggleNightVision(playerid, bool:toggle);
native ToggleThermalVision(playerid, bool:toggle);
native SAMPP_BindKey(playerid, key, event_mask = SAMPP_KEY_EVENT_DOWN, const action[] = "");
native SAMPP_UnbindKey(playerid, key);
native SAMPP_ClearKeyBinds(playerid);
native SAMPP_BeginKeyCapture(playerid, key, event_mask = SAMPP_KEY_EVENT_DOWN, priority = SAMPP_CAPTURE_PRIORITY_LOW, ttl_ms = SAMPP_CAPTURE_LEASE_DEFAULT_MS, flags = SAMPP_CAPTURE_DEFAULT_FLAGS, const action[] = "");
native SAMPP_EndKeyCapture(playerid, key, const action[] = "");
native SAMPP_ClearKeyCaptures(playerid);
native SAMPP_TargetBegin(playerid, targetid, const title[], ttl_ms = 500, flags = 0);
native SAMPP_TargetBeginEx(playerid, targetid, target_type, const title[], ttl_ms = 500, flags = 0);
native SAMPP_TargetSetLayout(playerid, targetid, layout);
native SAMPP_TargetSetDescription(playerid, targetid, const description[]);
native SAMPP_TargetAddOption(playerid, targetid, optionid, const label[], const icon[] = "", bool:enabled = true);
native SAMPP_TargetAddRow(playerid, targetid, optionid, row_type, const label[], const icon[] = "", bool:enabled = true);
native SAMPP_TargetCommit(playerid, targetid);
native SAMPP_TargetClear(playerid);
native SAMPP_BuildOpen(playerid, sessionid, const title[], Float:max_distance = 8.0);
native SAMPP_BuildClose(playerid);
native SAMPP_BuildClearParts(playerid);
native SAMPP_BuildAddPart(playerid, partid, modelid, const name[], const category[] = "", const cost[] = "");
native SAMPP_BuildSendResult(playerid, result, const message[]);
native SAMPP_BuildSetRemoveTarget(playerid, bool:active, partid = SAMPP_BUILD_PART_NONE, const label[] = "", Float:distance = 0.0);
native SAMPP_ExecuteCallback(type, {Float,_}:...);

stock bool:SAMPP_TargetAddAction(playerid, targetid, optionid, const label[], const icon[] = "")
{
	return SAMPP_TargetAddRow(playerid, targetid, optionid, SAMPP_TARGET_ROW_ACTION, label, icon, true) != 0;
}

stock bool:SAMPP_TargetAddInfo(playerid, targetid, const text[])
{
	return SAMPP_TargetAddRow(playerid, targetid, 0, SAMPP_TARGET_ROW_INFO, text, "", false) != 0;
}

stock bool:SAMPP_TargetAddDialog(playerid, targetid, const text[])
{
	return SAMPP_TargetAddRow(playerid, targetid, 0, SAMPP_TARGET_ROW_DIALOG, text, "", false) != 0;
}

stock bool:SAMPP_TargetAddDivider(playerid, targetid)
{
	return SAMPP_TargetAddRow(playerid, targetid, 0, SAMPP_TARGET_ROW_DIVIDER, "", "", false) != 0;
}

stock bool:SAMPP_TargetAddHeader(playerid, targetid, const text[])
{
	return SAMPP_TargetAddRow(playerid, targetid, 0, SAMPP_TARGET_ROW_HEADER, text, "", false) != 0;
}

stock bool:SAMPP_TargetAddDisabled(playerid, targetid, optionid, const label[], const icon[] = "")
{
	return SAMPP_TargetAddRow(playerid, targetid, optionid, SAMPP_TARGET_ROW_DISABLED, label, icon, false) != 0;
}

stock bool:SAMPP_TargetAddToggle(playerid, targetid, optionid, const label[], const icon[] = "")
{
	return SAMPP_TargetAddRow(playerid, targetid, optionid, SAMPP_TARGET_ROW_TOGGLE, label, icon, true) != 0;
}

stock bool:SAMPP_TargetAddDanger(playerid, targetid, optionid, const label[], const icon[] = "")
{
	return SAMPP_TargetAddRow(playerid, targetid, optionid, SAMPP_TARGET_ROW_DANGER, label, icon, true) != 0;
}

stock bool:SAMPP_TargetBeginDirect(playerid, targetid, target_type, const title[], ttl_ms = 500, flags = 0)
{
	return SAMPP_TargetBeginEx(playerid, targetid, target_type, title, ttl_ms, flags | SAMPP_TARGET_FLAG_DIRECT_SELECT) != 0;
}

stock bool:SAMPP_CaptureKeyNearPoint(playerid, key, Float:x, Float:y, Float:z, Float:radius, const action[], priority = SAMPP_CAPTURE_PRIORITY_ITEM, ttl_ms = SAMPP_CAPTURE_LEASE_DEFAULT_MS, flags = SAMPP_CAPTURE_DEFAULT_FLAGS, event_mask = SAMPP_KEY_EVENT_DOWN)
{
	if (!IsUsingSAMPP(playerid))
	{
		return false;
	}

	if (GetPlayerDistanceFromPoint(playerid, x, y, z) > radius)
	{
		return false;
	}

	return SAMPP_BeginKeyCapture(playerid, key, event_mask, priority, ttl_ms, flags, action) != 0;
}

stock bool:SAMPP_CaptureKeyInAnyVehicle(playerid, key, const action[], priority = SAMPP_CAPTURE_PRIORITY_VEHICLE, ttl_ms = SAMPP_CAPTURE_LEASE_DEFAULT_MS, flags = SAMPP_CAPTURE_DEFAULT_FLAGS, event_mask = SAMPP_KEY_EVENT_DOWN)
{
	if (!IsUsingSAMPP(playerid) || !IsPlayerInAnyVehicle(playerid))
	{
		return false;
	}

	return SAMPP_BeginKeyCapture(playerid, key, event_mask, priority, ttl_ms, flags, action) != 0;
}

stock bool:SAMPP_CaptureKeyInVehicle(playerid, vehicleid, key, const action[], priority = SAMPP_CAPTURE_PRIORITY_VEHICLE, ttl_ms = SAMPP_CAPTURE_LEASE_DEFAULT_MS, flags = SAMPP_CAPTURE_DEFAULT_FLAGS, event_mask = SAMPP_KEY_EVENT_DOWN)
{
	if (!IsUsingSAMPP(playerid) || !IsPlayerInVehicle(playerid, vehicleid))
	{
		return false;
	}

	return SAMPP_BeginKeyCapture(playerid, key, event_mask, priority, ttl_ms, flags, action) != 0;
}

stock bool:SAMPP_CaptureKeyNearVehicle(playerid, vehicleid, key, Float:radius, const action[], priority = SAMPP_CAPTURE_PRIORITY_VEHICLE, ttl_ms = SAMPP_CAPTURE_LEASE_DEFAULT_MS, flags = SAMPP_CAPTURE_DEFAULT_FLAGS, event_mask = SAMPP_KEY_EVENT_DOWN)
{
	if (!IsUsingSAMPP(playerid))
	{
		return false;
	}

	new Float:x, Float:y, Float:z;
	if (!GetVehiclePos(vehicleid, x, y, z))
	{
		return false;
	}

	return SAMPP_CaptureKeyNearPoint(playerid, key, x, y, z, radius, action, priority, ttl_ms, flags, event_mask);
}

stock bool:SAMPP_TargetHasUI(playerid)
{
	return IsUsingSAMPP(playerid) && SAMPP_HasFeature(playerid, SAMPP_FEATURE_TARGET);
}

stock bool:SAMPP_BuildHasUI(playerid)
{
	return IsUsingSAMPP(playerid) && SAMPP_HasFeature(playerid, SAMPP_FEATURE_BUILD);
}

stock bool:SAMPP_TargetBeginNearPoint(playerid, targetid, Float:x, Float:y, Float:z, Float:radius, const title[], ttl_ms = 500, flags = 0)
{
	if (!SAMPP_TargetHasUI(playerid))
	{
		return false;
	}

	if (GetPlayerDistanceFromPoint(playerid, x, y, z) > radius)
	{
		SAMPP_TargetClear(playerid);
		return false;
	}

	if (!SAMPP_TargetBegin(playerid, targetid, title, ttl_ms, flags))
	{
		return false;
	}

	return true;
}

forward OnPlayerOpenPauseMenu(playerid);
forward OnPlayerClosePauseMenu(playerid);
forward OnPlayerEnterPauseSubmenu(playerid, from, to);
forward OnDriverDriveByShot(playerid);
forward OnPlayerStunt(playerid, stuntid, money, details[]);
forward OnPlayerResolutionChange(playerid, X, Y);
forward OnPlayerSAMPPConnect(address[], port);
forward OnPlayerSAMPPJoin(playerid, bool:has_plugin);
forward OnPlayerClick(playerid, type, X, Y);
forward OnPlayerChangeRadioStation(playerid, stationid, vehicleid);
forward OnPlayerDrinkSprunk(playerid);
forward OnPlayerSAMPPKey(playerid, keyid, keystate, action[]);
forward OnPlayerOMPPlusReady(playerid);
forward OnPlayerOMPPlusKey(playerid, keyid, keystate, action[]);
forward OnPlayerSAMPPTargetMode(playerid, targetid, bool:opened);
forward OnPlayerSAMPPTargetSelect(playerid, targetid, optionid);
forward OnPlayerOMPPlusTargetMode(playerid, targetid, bool:opened);
forward OnPlayerOMPPlusTargetSelect(playerid, targetid, optionid);
forward OnPlayerSAMPPBuildSelect(playerid, sessionid, partid);
forward OnPlayerSAMPPBuildPlace(playerid, sessionid, partid, rotation_step, bool:flipped);
forward OnPlayerSAMPPBuildCancel(playerid, sessionid);
forward OnPlayerSAMPPBuildPreview(playerid, sessionid, partid, rotation_step, bool:flipped);
forward OnPlayerOMPPlusBuildSelect(playerid, sessionid, partid);
forward OnPlayerOMPPlusBuildPlace(playerid, sessionid, partid, rotation_step, bool:flipped);
forward OnPlayerOMPPlusBuildCancel(playerid, sessionid);
forward OnPlayerOMPPlusBuildPreview(playerid, sessionid, partid, rotation_step, bool:flipped);

public OnPlayerConnect(playerid)
{
	SAMPP_ExecuteCallback(0, playerid);

#if defined SAMPP_OnPlayerConnect
	return SAMPP_OnPlayerConnect(playerid);
#else
	return 1;
#endif
}

#if defined SAMPP_OnPlayerConnect
	forward SAMPP_OnPlayerConnect(playerid);
#endif
#if defined _ALS_OnPlayerConnect
	#undef OnPlayerConnect
#else
	#define _ALS_OnPlayerConnect
#endif
#define OnPlayerConnect SAMPP_OnPlayerConnect

public OnPlayerDisconnect(playerid, reason)
{
	SAMPP_ExecuteCallback(1, playerid, reason);

#if defined SAMPP_OnPlayerDisconnect
	return SAMPP_OnPlayerDisconnect(playerid, reason);
#else
	return 1;
#endif
}

#if defined SAMPP_OnPlayerDisconnect
	forward SAMPP_OnPlayerDisconnect(playerid, reason);
#endif
#if defined _ALS_OnPlayerDisconnect
	#undef OnPlayerDisconnect
#else
	#define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect SAMPP_OnPlayerDisconnect
