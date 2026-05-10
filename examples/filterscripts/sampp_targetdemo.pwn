#include <open.mp>
#include <sampp>

#define TARGET_DEMO_COLOUR 0x74D9FFFF
#define TARGET_DEMO_OK_COLOUR 0x9DFF86FF
#define TARGET_DEMO_WARN_COLOUR 0xFFB86CFF

#define TARGET_DEMO_VEHICLE_MODEL 445
#define TARGET_DEMO_RANGE 2.75
#define TARGET_DEMO_CLEAR_RANGE 3.20
#define TARGET_DEMO_TTL_MS 1200
#define TARGET_DEMO_REFRESH_MS 120
#define TARGET_DEMO_BASE_ID 910000

#define TARGET_OPTION_ENTER 1
#define TARGET_OPTION_ENGINE 2
#define TARGET_OPTION_HOOD 3
#define TARGET_OPTION_TRUNK 4
#define TARGET_OPTION_DOORS 5
#define TARGET_OPTION_LOCK 6

static gTargetVehicle[MAX_PLAYERS];
static bool:gTargetEngine[MAX_PLAYERS];
static bool:gTargetHood[MAX_PLAYERS];
static bool:gTargetTrunk[MAX_PLAYERS];
static bool:gTargetDoors[MAX_PLAYERS];
static bool:gTargetLocked[MAX_PLAYERS];
static bool:gTargetContextActive[MAX_PLAYERS];
static gTargetNextRefresh[MAX_PLAYERS];

stock TargetDemoId(playerid)
{
	return TARGET_DEMO_BASE_ID + playerid;
}

stock ResetTargetDemoPlayer(playerid)
{
	gTargetVehicle[playerid] = INVALID_VEHICLE_ID;
	gTargetEngine[playerid] = false;
	gTargetHood[playerid] = false;
	gTargetTrunk[playerid] = false;
	gTargetDoors[playerid] = false;
	gTargetLocked[playerid] = false;
	gTargetContextActive[playerid] = false;
	gTargetNextRefresh[playerid] = 0;
	return 1;
}

stock ClearTargetDemoContext(playerid)
{
	if (gTargetContextActive[playerid])
	{
		SAMPP_TargetClear(playerid);
		gTargetContextActive[playerid] = false;
	}
	return 1;
}

stock DestroyTargetDemoVehicle(playerid)
{
	if (gTargetVehicle[playerid] != INVALID_VEHICLE_ID)
	{
		DestroyVehicle(gTargetVehicle[playerid]);
	}
	gTargetVehicle[playerid] = INVALID_VEHICLE_ID;
	gTargetEngine[playerid] = false;
	gTargetHood[playerid] = false;
	gTargetTrunk[playerid] = false;
	gTargetDoors[playerid] = false;
	gTargetLocked[playerid] = false;
	return 1;
}

stock bool:HasTargetVehicle(playerid)
{
	return gTargetVehicle[playerid] != INVALID_VEHICLE_ID;
}

stock bool:IsLookingAtTargetVehicle(playerid)
{
	if (!HasTargetVehicle(playerid))
	{
		return false;
	}

	if (IsPlayerInVehicle(playerid, gTargetVehicle[playerid]))
	{
		return true;
	}

	if (IsPlayerInAnyVehicle(playerid))
	{
		return false;
	}

	new bool:cameraTargetMatches = GetPlayerCameraTargetVehicle(playerid) == gTargetVehicle[playerid];

	new Float:camX, Float:camY, Float:camZ;
	new Float:frontX, Float:frontY, Float:frontZ;
	new Float:vehX, Float:vehY, Float:vehZ;
	if (!GetPlayerCameraPos(playerid, camX, camY, camZ)
		|| !GetPlayerCameraFrontVector(playerid, frontX, frontY, frontZ)
		|| !GetVehiclePos(gTargetVehicle[playerid], vehX, vehY, vehZ))
	{
		return false;
	}

	new Float:toX = vehX - camX;
	new Float:toY = vehY - camY;
	new Float:toZ = vehZ - camZ;
	new Float:length = floatsqroot((toX * toX) + (toY * toY) + (toZ * toZ));
	if (length <= 0.001)
	{
		return false;
	}

	toX /= length;
	toY /= length;
	toZ /= length;

	new Float:dot = (toX * frontX) + (toY * frontY) + (toZ * frontZ);
	return dot >= (cameraTargetMatches ? 0.82 : 0.88);
}

stock bool:IsNearTargetVehicle(playerid)
{
	if (!HasTargetVehicle(playerid))
	{
		return false;
	}

	if (IsPlayerInVehicle(playerid, gTargetVehicle[playerid]))
	{
		return true;
	}

	if (IsPlayerInAnyVehicle(playerid))
	{
		return false;
	}

	new Float:x, Float:y, Float:z;
	if (!GetVehiclePos(gTargetVehicle[playerid], x, y, z))
	{
		return false;
	}

	return GetPlayerDistanceFromPoint(playerid, x, y, z) <= TARGET_DEMO_RANGE
		&& IsLookingAtTargetVehicle(playerid);
}

stock bool:ShouldForceClearTargetVehicle(playerid)
{
	if (!HasTargetVehicle(playerid))
	{
		return true;
	}

	if (IsPlayerInVehicle(playerid, gTargetVehicle[playerid]))
	{
		return false;
	}

	if (IsPlayerInAnyVehicle(playerid))
	{
		return true;
	}

	new Float:x, Float:y, Float:z;
	if (!GetVehiclePos(gTargetVehicle[playerid], x, y, z))
	{
		return true;
	}

	return GetPlayerDistanceFromPoint(playerid, x, y, z) > TARGET_DEMO_CLEAR_RANGE
		|| !IsLookingAtTargetVehicle(playerid);
}

stock bool:RequireTargetVehicle(playerid)
{
	if (!HasTargetVehicle(playerid))
	{
		SendClientMessage(playerid, TARGET_DEMO_WARN_COLOUR, "[TargetDemo] No demo vehicle. Use /targetveh.");
		return false;
	}

	if (!IsNearTargetVehicle(playerid))
	{
		SendClientMessage(playerid, TARGET_DEMO_WARN_COLOUR, "[TargetDemo] Move closer to the demo vehicle.");
		return false;
	}

	return true;
}

stock SyncVehicleParams(playerid)
{
	if (!HasTargetVehicle(playerid))
	{
		return 0;
	}

	new bool:engine, bool:lights, bool:alarm, bool:doors, bool:bonnet, bool:boot, bool:objective;
	GetVehicleParamsEx(gTargetVehicle[playerid], engine, lights, alarm, doors, bonnet, boot, objective);
	SetVehicleParamsEx(
		gTargetVehicle[playerid],
		gTargetEngine[playerid],
		lights,
		alarm,
		gTargetLocked[playerid],
		gTargetHood[playerid],
		gTargetTrunk[playerid],
		objective
	);
	SetVehicleParamsCarDoors(
		gTargetVehicle[playerid],
		gTargetDoors[playerid],
		gTargetDoors[playerid],
		gTargetDoors[playerid],
		gTargetDoors[playerid]
	);
	return 1;
}

stock SpawnTargetVehicle(playerid)
{
	DestroyTargetDemoVehicle(playerid);

	new Float:x, Float:y, Float:z, Float:a;
	if (!GetPlayerPos(playerid, x, y, z))
	{
		SendClientMessage(playerid, TARGET_DEMO_WARN_COLOUR, "[TargetDemo] Could not read your position.");
		return 1;
	}
	GetPlayerFacingAngle(playerid, a);

	gTargetVehicle[playerid] = CreateVehicle(TARGET_DEMO_VEHICLE_MODEL, x + 4.0, y + 1.5, z + 0.5, a, 1, 1, 60000);
	if (gTargetVehicle[playerid] == INVALID_VEHICLE_ID)
	{
		SendClientMessage(playerid, TARGET_DEMO_WARN_COLOUR, "[TargetDemo] Could not create the demo vehicle.");
		return 1;
	}

	SyncVehicleParams(playerid);
	SendClientMessage(playerid, TARGET_DEMO_OK_COLOUR, "[TargetDemo] Vehicle spawned. Move close to it, press ALT once, then select a target option.");
	SendClientMessage(playerid, TARGET_DEMO_COLOUR, "[TargetDemo] The menu is server-driven and every option is validated again on select.");
	return 1;
}

stock PublishTargetContext(playerid)
{
	if (!IsUsingSAMPP(playerid) || !SAMPP_HasFeature(playerid, SAMPP_FEATURE_TARGET))
	{
		ClearTargetDemoContext(playerid);
		return 1;
	}

	if (!IsNearTargetVehicle(playerid))
	{
		ClearTargetDemoContext(playerid);
		return 1;
	}

	new targetid = TargetDemoId(playerid);
	new flags = IsPlayerInVehicle(playerid, gTargetVehicle[playerid]) ? SAMPP_TARGET_FLAG_HIDE_PROMPT : 0;
	if (!SAMPP_TargetBeginEx(playerid, targetid, SAMPP_TARGET_TYPE_VEHICLE, "Demo Vehicle", TARGET_DEMO_TTL_MS, flags))
	{
		ClearTargetDemoContext(playerid);
		return 1;
	}
	SAMPP_TargetSetLayout(playerid, targetid, SAMPP_TARGET_LAYOUT_STANDARD);
	SAMPP_TargetSetDescription(playerid, targetid, "Manage this vehicle. Every action is validated by the server before it runs.");

	if (!IsPlayerInAnyVehicle(playerid))
	{
		SAMPP_TargetAddAction(playerid, targetid, TARGET_OPTION_ENTER, "Seat in Vehicle", "seat");
		SAMPP_TargetAddDivider(playerid, targetid);
	}

	SAMPP_TargetAddToggle(playerid, targetid, TARGET_OPTION_ENGINE, gTargetEngine[playerid] ? ("Turn Engine Off") : ("Turn Engine On"), "engine");
	SAMPP_TargetAddToggle(playerid, targetid, TARGET_OPTION_HOOD, gTargetHood[playerid] ? ("Close Hood") : ("Open Hood"), "hood");
	SAMPP_TargetAddToggle(playerid, targetid, TARGET_OPTION_TRUNK, gTargetTrunk[playerid] ? ("Close Trunk") : ("Open Trunk"), "trunk");
	SAMPP_TargetAddToggle(playerid, targetid, TARGET_OPTION_DOORS, gTargetDoors[playerid] ? ("Close Doors") : ("Open Doors"), "doors");
	SAMPP_TargetAddAction(playerid, targetid, TARGET_OPTION_LOCK, gTargetLocked[playerid] ? ("Unlock Vehicle") : ("Lock Vehicle"), "lock");

	SAMPP_TargetCommit(playerid, targetid);
	gTargetContextActive[playerid] = true;
	return 1;
}

stock RefreshTargetContext(playerid, bool:force = false)
{
	new now = GetTickCount();
	if (gTargetContextActive[playerid] && ShouldForceClearTargetVehicle(playerid))
	{
		ClearTargetDemoContext(playerid);
		gTargetNextRefresh[playerid] = now + TARGET_DEMO_REFRESH_MS;
		return 1;
	}

	if (!force && now < gTargetNextRefresh[playerid])
	{
		return 1;
	}
	gTargetNextRefresh[playerid] = now + TARGET_DEMO_REFRESH_MS;
	return PublishTargetContext(playerid);
}

stock RunTargetOption(playerid, optionid)
{
	if (!RequireTargetVehicle(playerid))
	{
		return 1;
	}

	switch (optionid)
	{
		case TARGET_OPTION_ENTER:
		{
			if (IsPlayerInAnyVehicle(playerid))
			{
				SendClientMessage(playerid, TARGET_DEMO_WARN_COLOUR, "[TargetDemo] You are already in a vehicle.");
				return 1;
			}

			PutPlayerInVehicle(playerid, gTargetVehicle[playerid], 0);
			SendClientMessage(playerid, TARGET_DEMO_OK_COLOUR, "[TargetDemo] Target action: seated in vehicle.");
		}
		case TARGET_OPTION_ENGINE:
		{
			gTargetEngine[playerid] = !gTargetEngine[playerid];
			SyncVehicleParams(playerid);
			SendClientMessage(playerid, TARGET_DEMO_OK_COLOUR, gTargetEngine[playerid] ? ("[TargetDemo] Engine on.") : ("[TargetDemo] Engine off."));
		}
		case TARGET_OPTION_HOOD:
		{
			gTargetHood[playerid] = !gTargetHood[playerid];
			SyncVehicleParams(playerid);
			SendClientMessage(playerid, TARGET_DEMO_OK_COLOUR, gTargetHood[playerid] ? ("[TargetDemo] Hood opened.") : ("[TargetDemo] Hood closed."));
		}
		case TARGET_OPTION_TRUNK:
		{
			gTargetTrunk[playerid] = !gTargetTrunk[playerid];
			SyncVehicleParams(playerid);
			SendClientMessage(playerid, TARGET_DEMO_OK_COLOUR, gTargetTrunk[playerid] ? ("[TargetDemo] Trunk opened.") : ("[TargetDemo] Trunk closed."));
		}
		case TARGET_OPTION_DOORS:
		{
			gTargetDoors[playerid] = !gTargetDoors[playerid];
			SyncVehicleParams(playerid);
			SendClientMessage(playerid, TARGET_DEMO_OK_COLOUR, gTargetDoors[playerid] ? ("[TargetDemo] Physical doors opened.") : ("[TargetDemo] Physical doors closed."));
		}
		case TARGET_OPTION_LOCK:
		{
			gTargetLocked[playerid] = !gTargetLocked[playerid];
			SyncVehicleParams(playerid);
			SendClientMessage(playerid, TARGET_DEMO_OK_COLOUR, gTargetLocked[playerid] ? ("[TargetDemo] Vehicle locked.") : ("[TargetDemo] Vehicle unlocked."));
		}
		default:
		{
			SendClientMessage(playerid, TARGET_DEMO_WARN_COLOUR, "[TargetDemo] Unknown target option.");
		}
	}

	ClearTargetDemoContext(playerid);
	gTargetNextRefresh[playerid] = GetTickCount() + TARGET_DEMO_REFRESH_MS;
	return 1;
}

stock SendTargetHelp(playerid)
{
	SendClientMessage(playerid, TARGET_DEMO_COLOUR, "[TargetDemo] /targetveh spawns a demo vehicle target.");
	SendClientMessage(playerid, TARGET_DEMO_COLOUR, "[TargetDemo] Stand close to it and press ALT once to open the ImGui target menu.");
	SendClientMessage(playerid, TARGET_DEMO_COLOUR, "[TargetDemo] /targetclear removes the demo vehicle. /targetinfo checks client feature support.");
	return 1;
}

stock SendTargetInfo(playerid)
{
	new message[128];
	format(message, sizeof message, "[TargetDemo] IsUsingSAMPP=%d TargetFeature=%d UIFeature=%d", IsUsingSAMPP(playerid), SAMPP_HasFeature(playerid, SAMPP_FEATURE_TARGET), SAMPP_HasFeature(playerid, SAMPP_FEATURE_UI));
	SendClientMessage(playerid, TARGET_DEMO_COLOUR, message);
	return 1;
}

public OnFilterScriptInit()
{
	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		ResetTargetDemoPlayer(playerid);
	}

	print("[sampp_targetdemo] loaded. Use /targetveh, then press ALT near the vehicle.");
	return 1;
}

public OnFilterScriptExit()
{
	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		ClearTargetDemoContext(playerid);
		DestroyTargetDemoVehicle(playerid);
	}

	print("[sampp_targetdemo] unloaded.");
	return 1;
}

public OnPlayerConnect(playerid)
{
	ResetTargetDemoPlayer(playerid);
	SendClientMessage(playerid, TARGET_DEMO_COLOUR, "[TargetDemo] Use /targetveh to test the FiveM-style target menu.");
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	ClearTargetDemoContext(playerid);
	DestroyTargetDemoVehicle(playerid);
	ResetTargetDemoPlayer(playerid);
	return 1;
}

public OnPlayerUpdate(playerid)
{
	RefreshTargetContext(playerid);
	return 1;
}

public OnPlayerStateChange(playerid, PLAYER_STATE:newstate, PLAYER_STATE:oldstate)
{
	if (newstate == PLAYER_STATE_DRIVER
		|| newstate == PLAYER_STATE_PASSENGER
		|| oldstate == PLAYER_STATE_DRIVER
		|| oldstate == PLAYER_STATE_PASSENGER)
	{
		RefreshTargetContext(playerid, true);
	}
	return 1;
}

public OnPlayerSAMPPJoin(playerid, bool:has_plugin)
{
	if (has_plugin)
	{
		SendTargetInfo(playerid);
	}
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (!strcmp(cmdtext, "/targethelp", true))
	{
		return SendTargetHelp(playerid);
	}

	if (!strcmp(cmdtext, "/targetinfo", true))
	{
		return SendTargetInfo(playerid);
	}

	if (!strcmp(cmdtext, "/targetveh", true) || !strcmp(cmdtext, "/targetvehicle", true))
	{
		return SpawnTargetVehicle(playerid);
	}

	if (!strcmp(cmdtext, "/targetclear", true))
	{
		ClearTargetDemoContext(playerid);
		DestroyTargetDemoVehicle(playerid);
		SendClientMessage(playerid, TARGET_DEMO_COLOUR, "[TargetDemo] Demo target cleared.");
		return 1;
	}

	return 0;
}

public OnPlayerOMPPlusTargetMode(playerid, targetid, bool:opened)
{
	return 1;
}

public OnPlayerOMPPlusTargetSelect(playerid, targetid, optionid)
{
	if (targetid != TargetDemoId(playerid))
	{
		return 1;
	}

	return RunTargetOption(playerid, optionid);
}
