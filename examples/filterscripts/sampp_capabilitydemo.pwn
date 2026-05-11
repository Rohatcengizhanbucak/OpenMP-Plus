#include <open.mp>
#include <sampp>
#include <sampp_builddemo_core>

#define CAP_DEMO_MODEL_WATER_BOTTLE 19570
#define CAP_DEMO_MODEL_VEHICLE 411
#define CAP_DEMO_ITEM_RANGE 2.0
#define CAP_DEMO_VEHICLE_RANGE 4.0
#define CAP_DEMO_CAPTURE_TTL_MS 450
#define CAP_DEMO_CAPTURE_REFRESH_MS 150
#define CAP_DEMO_OBJECT_DRAW_DISTANCE 50.0
#define CAP_DEMO_LABEL_DRAW_DISTANCE 16.0

#define CAP_DEMO_COLOUR 0x74D9FFFF
#define CAP_DEMO_WARN_COLOUR 0xFFB86CFF
#define CAP_DEMO_OK_COLOUR 0x9DFF86FF

#define CAP_MODE_NONE 0
#define CAP_MODE_ITEM 1
#define CAP_MODE_VEHICLE 2
#define CAP_MODE_ENGINE 3

#define CAP_ACTION_ITEM "cap_item"
#define CAP_ACTION_VEHICLE "cap_vehicle"
#define CAP_ACTION_ENGINE "cap_engine"
#define CAP_ACTION_BONNET "cap_bonnet"
#define CAP_ACTION_BOOT "cap_boot"
#define CAP_ACTION_DOORS "cap_doors"
#define CAP_ACTION_LOCK "cap_lock"

static bool:gCapItemActive[MAX_PLAYERS];
static gCapItemObject[MAX_PLAYERS];
static Text3D:gCapItemLabel[MAX_PLAYERS];
static Float:gCapItemX[MAX_PLAYERS];
static Float:gCapItemY[MAX_PLAYERS];
static Float:gCapItemZ[MAX_PLAYERS];
static gCapItemWorld[MAX_PLAYERS];
static gCapItemInterior[MAX_PLAYERS];

static gCapVehicle[MAX_PLAYERS];
static bool:gCapEngineState[MAX_PLAYERS];
static bool:gCapDoorLockState[MAX_PLAYERS];
static bool:gCapBonnetState[MAX_PLAYERS];
static bool:gCapBootState[MAX_PLAYERS];
static bool:gCapCarDoorsState[MAX_PLAYERS];
static bool:gCapVehiclePartCapturesActive[MAX_PLAYERS];
static gCapMode[MAX_PLAYERS];
static gCapNextRefresh[MAX_PLAYERS];

stock CapResetPlayerState(playerid)
{
	gCapItemActive[playerid] = false;
	gCapItemObject[playerid] = INVALID_OBJECT_ID;
	gCapItemLabel[playerid] = INVALID_3DTEXT_ID;
	gCapItemX[playerid] = 0.0;
	gCapItemY[playerid] = 0.0;
	gCapItemZ[playerid] = 0.0;
	gCapItemWorld[playerid] = 0;
	gCapItemInterior[playerid] = 0;

	gCapVehicle[playerid] = INVALID_VEHICLE_ID;
	gCapEngineState[playerid] = false;
	gCapDoorLockState[playerid] = false;
	gCapBonnetState[playerid] = false;
	gCapBootState[playerid] = false;
	gCapCarDoorsState[playerid] = false;
	gCapVehiclePartCapturesActive[playerid] = false;
	gCapMode[playerid] = CAP_MODE_NONE;
	gCapNextRefresh[playerid] = 0;
	return 1;
}

stock CapDestroyItem(playerid)
{
	if (gCapItemObject[playerid] != INVALID_OBJECT_ID)
	{
		DestroyObject(gCapItemObject[playerid]);
	}

	if (gCapItemLabel[playerid] != INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(gCapItemLabel[playerid]);
	}

	gCapItemActive[playerid] = false;
	gCapItemObject[playerid] = INVALID_OBJECT_ID;
	gCapItemLabel[playerid] = INVALID_3DTEXT_ID;
	return 1;
}

stock CapDestroyVehicle(playerid)
{
	if (gCapVehicle[playerid] != INVALID_VEHICLE_ID)
	{
		DestroyVehicle(gCapVehicle[playerid]);
	}

	gCapVehicle[playerid] = INVALID_VEHICLE_ID;
	gCapEngineState[playerid] = false;
	gCapDoorLockState[playerid] = false;
	gCapBonnetState[playerid] = false;
	gCapBootState[playerid] = false;
	gCapCarDoorsState[playerid] = false;
	return 1;
}

stock CapEndMode(playerid, mode)
{
	switch (mode)
	{
		case CAP_MODE_ITEM:
		{
			SAMPP_EndKeyCapture(playerid, SAMPP_KEY_E, CAP_ACTION_ITEM);
		}
		case CAP_MODE_VEHICLE:
		{
			SAMPP_EndKeyCapture(playerid, SAMPP_KEY_E, CAP_ACTION_VEHICLE);
		}
		case CAP_MODE_ENGINE:
		{
			SAMPP_EndKeyCapture(playerid, SAMPP_KEY_E, CAP_ACTION_ENGINE);
		}
	}
	return 1;
}

stock CapClearCaptures(playerid)
{
	CapEndMode(playerid, CAP_MODE_ITEM);
	CapEndMode(playerid, CAP_MODE_VEHICLE);
	CapEndMode(playerid, CAP_MODE_ENGINE);
	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_H, CAP_ACTION_BONNET);
	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_J, CAP_ACTION_BOOT);
	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_K, CAP_ACTION_DOORS);
	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_L, CAP_ACTION_LOCK);
	gCapVehiclePartCapturesActive[playerid] = false;
	gCapMode[playerid] = CAP_MODE_NONE;
	return 1;
}

stock bool:CapHasNativeCapture(playerid)
{
	return IsUsingSAMPP(playerid) && SAMPP_HasFeature(playerid, SAMPP_FEATURE_KEYCAPTURE);
}

stock bool:CapIsNearItem(playerid)
{
	if (!gCapItemActive[playerid])
	{
		return false;
	}

	if (GetPlayerVirtualWorld(playerid) != gCapItemWorld[playerid] || GetPlayerInterior(playerid) != gCapItemInterior[playerid])
	{
		return false;
	}

	return GetPlayerDistanceFromPoint(playerid, gCapItemX[playerid], gCapItemY[playerid], gCapItemZ[playerid]) <= CAP_DEMO_ITEM_RANGE;
}

stock bool:CapIsNearVehicle(playerid)
{
	if (gCapVehicle[playerid] == INVALID_VEHICLE_ID)
	{
		return false;
	}

	new Float:x, Float:y, Float:z;
	if (!GetVehiclePos(gCapVehicle[playerid], x, y, z))
	{
		return false;
	}

	return GetPlayerDistanceFromPoint(playerid, x, y, z) <= CAP_DEMO_VEHICLE_RANGE;
}

stock CapResolveMode(playerid)
{
	if (!CapHasNativeCapture(playerid))
	{
		return CAP_MODE_NONE;
	}

	if (gCapVehicle[playerid] != INVALID_VEHICLE_ID && IsPlayerInVehicle(playerid, gCapVehicle[playerid]))
	{
		return CAP_MODE_ENGINE;
	}

	if (IsPlayerInAnyVehicle(playerid))
	{
		return CAP_MODE_NONE;
	}

	if (CapIsNearVehicle(playerid))
	{
		return CAP_MODE_VEHICLE;
	}

	if (CapIsNearItem(playerid))
	{
		return CAP_MODE_ITEM;
	}

	return CAP_MODE_NONE;
}

stock CapBeginMode(playerid, mode)
{
	switch (mode)
	{
		case CAP_MODE_ITEM:
		{
			SAMPP_BeginKeyCapture(
				playerid,
				SAMPP_KEY_E,
				SAMPP_KEY_EVENT_DOWN,
				SAMPP_CAPTURE_PRIORITY_ITEM,
				CAP_DEMO_CAPTURE_TTL_MS,
				SAMPP_CAPTURE_DEFAULT_FLAGS,
				CAP_ACTION_ITEM
			);
		}
		case CAP_MODE_VEHICLE:
		{
			SAMPP_BeginKeyCapture(
				playerid,
				SAMPP_KEY_E,
				SAMPP_KEY_EVENT_DOWN,
				SAMPP_CAPTURE_PRIORITY_VEHICLE,
				CAP_DEMO_CAPTURE_TTL_MS,
				SAMPP_CAPTURE_DEFAULT_FLAGS,
				CAP_ACTION_VEHICLE
			);
		}
		case CAP_MODE_ENGINE:
		{
			SAMPP_BeginKeyCapture(
				playerid,
				SAMPP_KEY_E,
				SAMPP_KEY_EVENT_DOWN,
				SAMPP_CAPTURE_PRIORITY_MENU,
				CAP_DEMO_CAPTURE_TTL_MS,
				SAMPP_CAPTURE_DEFAULT_FLAGS,
				CAP_ACTION_ENGINE
			);
		}
	}
	return 1;
}

stock CapBeginVehiclePartCaptures(playerid)
{
	SAMPP_BeginKeyCapture(playerid, SAMPP_KEY_H, SAMPP_KEY_EVENT_DOWN, SAMPP_CAPTURE_PRIORITY_MENU, CAP_DEMO_CAPTURE_TTL_MS, SAMPP_CAPTURE_DEFAULT_FLAGS, CAP_ACTION_BONNET);
	SAMPP_BeginKeyCapture(playerid, SAMPP_KEY_J, SAMPP_KEY_EVENT_DOWN, SAMPP_CAPTURE_PRIORITY_MENU, CAP_DEMO_CAPTURE_TTL_MS, SAMPP_CAPTURE_DEFAULT_FLAGS, CAP_ACTION_BOOT);
	SAMPP_BeginKeyCapture(playerid, SAMPP_KEY_K, SAMPP_KEY_EVENT_DOWN, SAMPP_CAPTURE_PRIORITY_MENU, CAP_DEMO_CAPTURE_TTL_MS, SAMPP_CAPTURE_DEFAULT_FLAGS, CAP_ACTION_DOORS);
	SAMPP_BeginKeyCapture(playerid, SAMPP_KEY_L, SAMPP_KEY_EVENT_DOWN, SAMPP_CAPTURE_PRIORITY_MENU, CAP_DEMO_CAPTURE_TTL_MS, SAMPP_CAPTURE_DEFAULT_FLAGS, CAP_ACTION_LOCK);
	gCapVehiclePartCapturesActive[playerid] = true;
	return 1;
}

stock CapEndVehiclePartCaptures(playerid)
{
	if (!gCapVehiclePartCapturesActive[playerid])
	{
		return 1;
	}

	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_H, CAP_ACTION_BONNET);
	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_J, CAP_ACTION_BOOT);
	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_K, CAP_ACTION_DOORS);
	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_L, CAP_ACTION_LOCK);
	gCapVehiclePartCapturesActive[playerid] = false;
	return 1;
}

stock CapRefreshContext(playerid, bool:force = false)
{
	new now = GetTickCount();
	if (!force && now < gCapNextRefresh[playerid])
	{
		return 1;
	}
	gCapNextRefresh[playerid] = now + CAP_DEMO_CAPTURE_REFRESH_MS;

	new nextMode = CapResolveMode(playerid);
	if (nextMode != gCapMode[playerid])
	{
		CapEndMode(playerid, gCapMode[playerid]);
		gCapMode[playerid] = nextMode;
	}

	if (nextMode != CAP_MODE_NONE)
	{
		CapBeginMode(playerid, nextMode);
	}

	if (nextMode == CAP_MODE_ENGINE)
	{
		CapBeginVehiclePartCaptures(playerid);
	}
	else
	{
		CapEndVehiclePartCaptures(playerid);
	}

	return 1;
}

stock CapSendFeatureLine(playerid)
{
	new message[144];
	format(
		message,
		sizeof message,
		"[CapDemo] Features: HUD=%d Keybind=%d Capture=%d Target=%d Build=%d Audio=%d Effects=%d UI=%d",
		SAMPP_HasFeature(playerid, SAMPP_FEATURE_HUD),
		SAMPP_HasFeature(playerid, SAMPP_FEATURE_KEYBIND),
		SAMPP_HasFeature(playerid, SAMPP_FEATURE_KEYCAPTURE),
		SAMPP_HasFeature(playerid, SAMPP_FEATURE_TARGET),
		SAMPP_HasFeature(playerid, SAMPP_FEATURE_BUILD),
		SAMPP_HasFeature(playerid, SAMPP_FEATURE_AUDIO),
		SAMPP_HasFeature(playerid, SAMPP_FEATURE_EFFECTS),
		SAMPP_HasFeature(playerid, SAMPP_FEATURE_UI)
	);
	SendClientMessage(playerid, CAP_DEMO_COLOUR, message);
	return 1;
}

stock CapSendInfo(playerid)
{
	new message[144];
	format(message, sizeof message, "[CapDemo] IsUsingSAMPP=%d IsUsingOMPPlus=%d", IsUsingSAMPP(playerid), IsUsingOMPPlus(playerid));
	SendClientMessage(playerid, CAP_DEMO_COLOUR, message);

	if (!IsUsingSAMPP(playerid))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] Client extension missing. Use command fallbacks only; no E capture lease is issued.");
		return 1;
	}

	new major, minor, patch;
	new featureFlags = SAMPP_GetClientFeatureFlags(playerid);
	new capabilities = SAMPP_GetClientCapabilities(playerid);
	SAMPP_GetClientVersion(playerid, major, minor, patch);

	format(
		message,
		sizeof message,
		"[CapDemo] Client version=%d.%d.%d featureFlags=0x%x capabilities=0x%x verified=%d",
		major,
		minor,
		patch,
		featureFlags,
		capabilities,
		SAMPP_IsLauncherVerified(playerid)
	);
	SendClientMessage(playerid, CAP_DEMO_COLOUR, message);
	CapSendFeatureLine(playerid);

	new hash[65];
	SAMPP_GetClientHash(playerid, hash, sizeof hash);
	if (hash[0])
	{
		hash[20] = '\0';
		format(message, sizeof message, "[CapDemo] Client hash prefix: %s...", hash);
		SendClientMessage(playerid, CAP_DEMO_COLOUR, message);
	}
	else
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] Client hash was not reported by this client.");
	}

	if (!SAMPP_HasFeature(playerid, SAMPP_FEATURE_KEYCAPTURE))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] KEYCAPTURE unsupported. Use /cappickup, /capenter, /capengine fallback commands.");
	}
	return 1;
}

stock CapCreateItem(playerid)
{
	CapDestroyItem(playerid);

	new Float:x, Float:y, Float:z;
	if (!GetPlayerPos(playerid, x, y, z))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] Could not read your position.");
		return 1;
	}

	gCapItemX[playerid] = x + 1.5;
	gCapItemY[playerid] = y;
	gCapItemZ[playerid] = z - 0.85;
	gCapItemWorld[playerid] = GetPlayerVirtualWorld(playerid);
	gCapItemInterior[playerid] = GetPlayerInterior(playerid);

	gCapItemObject[playerid] = CreateObject(
		CAP_DEMO_MODEL_WATER_BOTTLE,
		gCapItemX[playerid],
		gCapItemY[playerid],
		gCapItemZ[playerid],
		0.0,
		0.0,
		0.0,
		CAP_DEMO_OBJECT_DRAW_DISTANCE
	);

	if (gCapItemObject[playerid] == INVALID_OBJECT_ID)
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] Could not create the item object.");
		return 1;
	}

	gCapItemLabel[playerid] = Create3DTextLabel(
		"{74D9FF}Capability Item\n{FFFFFF}Press E near it",
		CAP_DEMO_COLOUR,
		gCapItemX[playerid],
		gCapItemY[playerid],
		gCapItemZ[playerid] + 1.05,
		CAP_DEMO_LABEL_DRAW_DISTANCE,
		gCapItemWorld[playerid],
		false
	);

	if (gCapItemLabel[playerid] == INVALID_3DTEXT_ID)
	{
		DestroyObject(gCapItemObject[playerid]);
		gCapItemObject[playerid] = INVALID_OBJECT_ID;
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] Could not create the item label.");
		return 1;
	}

	gCapItemActive[playerid] = true;
	SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Item spawned. E is captured only while you are next to it.");
	CapRefreshContext(playerid, true);
	return 1;
}

stock CapCreateVehicle(playerid)
{
	CapDestroyVehicle(playerid);

	new Float:x, Float:y, Float:z, Float:a;
	if (!GetPlayerPos(playerid, x, y, z))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] Could not read your position.");
		return 1;
	}
	GetPlayerFacingAngle(playerid, a);

	gCapVehicle[playerid] = CreateVehicle(CAP_DEMO_MODEL_VEHICLE, x + 4.0, y + 1.0, z + 0.5, a, 1, 1, 60000);
	if (gCapVehicle[playerid] == INVALID_VEHICLE_ID)
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] Could not create the demo vehicle.");
		return 1;
	}

	new bool:engine, bool:lights, bool:alarm, bool:doors, bool:bonnet, bool:boot, bool:objective;
	GetVehicleParamsEx(gCapVehicle[playerid], engine, lights, alarm, doors, bonnet, boot, objective);
	SetVehicleParamsEx(gCapVehicle[playerid], false, lights, alarm, false, false, false, objective);
	SetVehicleParamsCarDoors(gCapVehicle[playerid], 0, 0, 0, 0);
	gCapEngineState[playerid] = false;
	gCapDoorLockState[playerid] = false;
	gCapBonnetState[playerid] = false;
	gCapBootState[playerid] = false;
	gCapCarDoorsState[playerid] = false;

	SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Vehicle spawned. E near it enters; inside: E engine, H hood, J trunk, K doors, L lock.");
	CapRefreshContext(playerid, true);
	return 1;
}

stock CapSpawnScenario(playerid)
{
	CapCreateItem(playerid);
	CapCreateVehicle(playerid);
	SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] Priority rule: vehicle context wins over item; in-vehicle engine context wins over both.");
	return 1;
}

stock CapPickupItem(playerid)
{
	if (!gCapItemActive[playerid] || !CapIsNearItem(playerid))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] No capability item nearby.");
		return 1;
	}

	CapDestroyItem(playerid);
	CapEndMode(playerid, CAP_MODE_ITEM);
	if (gCapMode[playerid] == CAP_MODE_ITEM)
	{
		gCapMode[playerid] = CAP_MODE_NONE;
	}

	ApplyAnimation(playerid, "BOMBER", "BOM_Plant", 4.0, false, false, false, false, 0);
	SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Picked up item through contextual E interaction.");
	return 1;
}

stock CapEnterVehicle(playerid)
{
	if (gCapVehicle[playerid] == INVALID_VEHICLE_ID || !CapIsNearVehicle(playerid))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] Demo vehicle is not nearby.");
		return 1;
	}

	if (IsPlayerInAnyVehicle(playerid))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] You are already in a vehicle.");
		return 1;
	}

	PutPlayerInVehicle(playerid, gCapVehicle[playerid], 0);
	SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Entered demo vehicle through contextual E interaction.");
	CapRefreshContext(playerid, true);
	return 1;
}

stock CapToggleEngine(playerid)
{
	if (gCapVehicle[playerid] == INVALID_VEHICLE_ID || !IsPlayerInVehicle(playerid, gCapVehicle[playerid]))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] You must be inside the demo vehicle.");
		return 1;
	}

	new bool:engine, bool:lights, bool:alarm, bool:doors, bool:bonnet, bool:boot, bool:objective;
	GetVehicleParamsEx(gCapVehicle[playerid], engine, lights, alarm, doors, bonnet, boot, objective);
	engine = !gCapEngineState[playerid];
	SetVehicleParamsEx(gCapVehicle[playerid], engine, lights, alarm, gCapDoorLockState[playerid], gCapBonnetState[playerid], gCapBootState[playerid], objective);
	gCapEngineState[playerid] = engine;

	if (engine)
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Demo engine action: enabled.");
	}
	else
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Demo engine action: disabled.");
	}
	return 1;
}

stock CapToggleBonnet(playerid)
{
	if (gCapVehicle[playerid] == INVALID_VEHICLE_ID || !IsPlayerInVehicle(playerid, gCapVehicle[playerid]))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] You must be inside the demo vehicle.");
		return 1;
	}

	new bool:engine, bool:lights, bool:alarm, bool:doors, bool:bonnet, bool:boot, bool:objective;
	GetVehicleParamsEx(gCapVehicle[playerid], engine, lights, alarm, doors, bonnet, boot, objective);
	gCapBonnetState[playerid] = !gCapBonnetState[playerid];
	SetVehicleParamsEx(gCapVehicle[playerid], gCapEngineState[playerid], lights, alarm, gCapDoorLockState[playerid], gCapBonnetState[playerid], gCapBootState[playerid], objective);

	if (gCapBonnetState[playerid])
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Hood/bonnet opened.");
	}
	else
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Hood/bonnet closed.");
	}
	return 1;
}

stock CapToggleBoot(playerid)
{
	if (gCapVehicle[playerid] == INVALID_VEHICLE_ID || !IsPlayerInVehicle(playerid, gCapVehicle[playerid]))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] You must be inside the demo vehicle.");
		return 1;
	}

	new bool:engine, bool:lights, bool:alarm, bool:doors, bool:bonnet, bool:boot, bool:objective;
	GetVehicleParamsEx(gCapVehicle[playerid], engine, lights, alarm, doors, bonnet, boot, objective);
	gCapBootState[playerid] = !gCapBootState[playerid];
	SetVehicleParamsEx(gCapVehicle[playerid], gCapEngineState[playerid], lights, alarm, gCapDoorLockState[playerid], gCapBonnetState[playerid], gCapBootState[playerid], objective);

	if (gCapBootState[playerid])
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Trunk/boot opened.");
	}
	else
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Trunk/boot closed.");
	}
	return 1;
}

stock CapToggleCarDoors(playerid)
{
	if (gCapVehicle[playerid] == INVALID_VEHICLE_ID || !IsPlayerInVehicle(playerid, gCapVehicle[playerid]))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] You must be inside the demo vehicle.");
		return 1;
	}

	gCapCarDoorsState[playerid] = !gCapCarDoorsState[playerid];
	SetVehicleParamsCarDoors(
		gCapVehicle[playerid],
		gCapCarDoorsState[playerid],
		gCapCarDoorsState[playerid],
		gCapCarDoorsState[playerid],
		gCapCarDoorsState[playerid]
	);

	if (gCapCarDoorsState[playerid])
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Physical car doors opened.");
	}
	else
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Physical car doors closed.");
	}
	return 1;
}

stock CapToggleDoorLock(playerid)
{
	if (gCapVehicle[playerid] == INVALID_VEHICLE_ID || !IsPlayerInVehicle(playerid, gCapVehicle[playerid]))
	{
		SendClientMessage(playerid, CAP_DEMO_WARN_COLOUR, "[CapDemo] You must be inside the demo vehicle.");
		return 1;
	}

	new bool:engine, bool:lights, bool:alarm, bool:doors, bool:bonnet, bool:boot, bool:objective;
	GetVehicleParamsEx(gCapVehicle[playerid], engine, lights, alarm, doors, bonnet, boot, objective);
	gCapDoorLockState[playerid] = !gCapDoorLockState[playerid];
	SetVehicleParamsEx(gCapVehicle[playerid], gCapEngineState[playerid], lights, alarm, gCapDoorLockState[playerid], gCapBonnetState[playerid], gCapBootState[playerid], objective);

	if (gCapDoorLockState[playerid])
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Vehicle doors locked.");
	}
	else
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] Vehicle doors unlocked.");
	}
	return 1;
}

stock CapSendHelp(playerid)
{
	SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] /capinfo shows negotiated client version, hash, features, and launcher flag.");
	SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] /capspawn creates one item and one vehicle. /capitem and /capveh create them separately.");
	SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] E near item=pickup, E near vehicle=enter, E inside demo vehicle=engine action.");
	SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] Inside demo vehicle: H hood, J trunk, K physical doors, L lock/unlock.");
	SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] Fallback commands: /cappickup, /capenter, /capengine, /capbonnet, /capboot, /capdoors, /caplock, /capclear.");
	return 1;
}

public OnFilterScriptInit()
{
	RegisterBuildDemoModels();

	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		CapResetPlayerState(playerid);
		ResetBuildDemoPlayer(playerid);
	}

	print("[sampp_capabilitydemo] loaded. Use /caphelp and /capspawn in-game.");
	print("[sampp_capabilitydemo] build demo bridge loaded. Use /builddemo in-game.");
	return 1;
}

public OnFilterScriptExit()
{
	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		CapClearCaptures(playerid);
		CapDestroyItem(playerid);
		CapDestroyVehicle(playerid);
		DestroyBuildDemoObjects(playerid);
	}

	print("[sampp_capabilitydemo] unloaded.");
	return 1;
}

public OnPlayerConnect(playerid)
{
	CapResetPlayerState(playerid);
	ResetBuildDemoPlayer(playerid);
	SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] Use /capinfo and /capspawn to test capabilities and contextual E capture.");
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	CapClearCaptures(playerid);
	CapDestroyItem(playerid);
	CapDestroyVehicle(playerid);
	CloseBuildDemo(playerid);
	DestroyBuildDemoObjects(playerid);
	ResetBuildDemoPlayer(playerid);
	CapResetPlayerState(playerid);
	return 1;
}

public OnPlayerUpdate(playerid)
{
	CapRefreshContext(playerid);
	if (IsBuildDemoActive(playerid))
	{
		RefreshBuildPreview(playerid);
	}
	return 1;
}

public OnPlayerSAMPPJoin(playerid, bool:has_plugin)
{
	if (has_plugin)
	{
		SendClientMessage(playerid, CAP_DEMO_OK_COLOUR, "[CapDemo] OMP+ client ready. /capinfo shows negotiated features.");
		CapSendInfo(playerid);
		SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Build UI bridge is loaded through CapDemo. Use /builddemo.");
	}
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (!strcmp(cmdtext, "/buildhelp", true))
	{
		return SendBuildDemoHelp(playerid);
	}

	if (!strcmp(cmdtext, "/builddemo", true) || !strcmp(cmdtext, "/build", true))
	{
		return OpenBuildDemo(playerid);
	}

	if (!strcmp(cmdtext, "/buildclose", true))
	{
		CloseBuildDemo(playerid);
		SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Build UI closed.");
		return 1;
	}

	if (!strcmp(cmdtext, "/buildclear", true))
	{
		CloseBuildDemo(playerid);
		DestroyBuildDemoObjects(playerid);
		SendClientMessage(playerid, BUILD_DEMO_COLOUR, "[BuildDemo] Demo build objects cleared.");
		return 1;
	}

	if (!strcmp(cmdtext, "/caphelp", true))
	{
		return CapSendHelp(playerid);
	}

	if (!strcmp(cmdtext, "/capinfo", true) || !strcmp(cmdtext, "/ompinfo", true))
	{
		return CapSendInfo(playerid);
	}

	if (!strcmp(cmdtext, "/capspawn", true))
	{
		return CapSpawnScenario(playerid);
	}

	if (!strcmp(cmdtext, "/capitem", true))
	{
		return CapCreateItem(playerid);
	}

	if (!strcmp(cmdtext, "/capveh", true) || !strcmp(cmdtext, "/capvehicle", true))
	{
		return CapCreateVehicle(playerid);
	}

	if (!strcmp(cmdtext, "/cappickup", true))
	{
		return CapPickupItem(playerid);
	}

	if (!strcmp(cmdtext, "/capenter", true))
	{
		return CapEnterVehicle(playerid);
	}

	if (!strcmp(cmdtext, "/capengine", true))
	{
		return CapToggleEngine(playerid);
	}

	if (!strcmp(cmdtext, "/capbonnet", true) || !strcmp(cmdtext, "/caphood", true) || !strcmp(cmdtext, "/capkaput", true))
	{
		return CapToggleBonnet(playerid);
	}

	if (!strcmp(cmdtext, "/capboot", true) || !strcmp(cmdtext, "/captrunk", true) || !strcmp(cmdtext, "/capbagaj", true))
	{
		return CapToggleBoot(playerid);
	}

	if (!strcmp(cmdtext, "/capdoors", true) || !strcmp(cmdtext, "/capkapi", true))
	{
		return CapToggleCarDoors(playerid);
	}

	if (!strcmp(cmdtext, "/caplock", true) || !strcmp(cmdtext, "/capkilit", true))
	{
		return CapToggleDoorLock(playerid);
	}

	if (!strcmp(cmdtext, "/capclear", true))
	{
		CapClearCaptures(playerid);
		CapDestroyItem(playerid);
		CapDestroyVehicle(playerid);
		SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] Your demo item, vehicle, and capture leases were cleared.");
		return 1;
	}

	return 0;
}

public OnPlayerOMPPlusBuildSelect(playerid, sessionid, partid)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	SetBuildDemoSelection(playerid, partid, 0, false, true);

	new message[104];
	format(message, sizeof message, "[BuildDemo] Selected part id %d. Move your aim, then LMB confirms the preview.", partid);
	SendClientMessage(playerid, BUILD_DEMO_COLOUR, message);
	return 1;
}

public OnPlayerOMPPlusBuildPreview(playerid, sessionid, partid, rotation_step, bool:flipped)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	SetBuildDemoSelection(playerid, partid, rotation_step, flipped, true);
	return 1;
}

public OnPlayerOMPPlusBuildPlace(playerid, sessionid, partid, rotation_step, bool:flipped)
{
	if (sessionid != GetBuildDemoSession(playerid))
	{
		return 1;
	}

	return PlaceBuildDemoPart(playerid, partid, rotation_step, flipped);
}

public OnPlayerOMPPlusBuildCancel(playerid, sessionid)
{
	if (sessionid == GetBuildDemoSession(playerid))
	{
		CancelBuildDemoSession(playerid, true);
	}
	return 1;
}

public OnPlayerSAMPPKey(playerid, keyid, keystate, action[])
{
	if (keystate != SAMPP_KEY_STATE_DOWN)
	{
		return 1;
	}

	if (keyid == SAMPP_KEY_E && !strcmp(action, CAP_ACTION_ITEM, true))
	{
		SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] E callback action=cap_item");
		return CapPickupItem(playerid);
	}

	if (keyid == SAMPP_KEY_E && !strcmp(action, CAP_ACTION_VEHICLE, true))
	{
		SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] E callback action=cap_vehicle");
		return CapEnterVehicle(playerid);
	}

	if (keyid == SAMPP_KEY_E && !strcmp(action, CAP_ACTION_ENGINE, true))
	{
		SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] E callback action=cap_engine");
		return CapToggleEngine(playerid);
	}

	if (keyid == SAMPP_KEY_H && !strcmp(action, CAP_ACTION_BONNET, true))
	{
		SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] H callback action=cap_bonnet");
		return CapToggleBonnet(playerid);
	}

	if (keyid == SAMPP_KEY_J && !strcmp(action, CAP_ACTION_BOOT, true))
	{
		SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] J callback action=cap_boot");
		return CapToggleBoot(playerid);
	}

	if (keyid == SAMPP_KEY_K && !strcmp(action, CAP_ACTION_DOORS, true))
	{
		SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] K callback action=cap_doors");
		return CapToggleCarDoors(playerid);
	}

	if (keyid == SAMPP_KEY_L && !strcmp(action, CAP_ACTION_LOCK, true))
	{
		SendClientMessage(playerid, CAP_DEMO_COLOUR, "[CapDemo] L callback action=cap_lock");
		return CapToggleDoorLock(playerid);
	}

	return 1;
}
