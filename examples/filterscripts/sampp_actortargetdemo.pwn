#include <open.mp>
#include <sampp>

#define ACTOR_DEMO_COLOUR 0x74D9FFFF
#define ACTOR_DEMO_OK_COLOUR 0x9DFF86FF
#define ACTOR_DEMO_WARN_COLOUR 0xFFB86CFF

#define ACTOR_DEMO_SKIN 276
#define ACTOR_DEMO_RANGE 2.75
#define ACTOR_DEMO_CLEAR_RANGE 3.20
#define ACTOR_DEMO_TTL_MS 1200
#define ACTOR_DEMO_REFRESH_MS 120
#define ACTOR_DEMO_BASE_ID 920000

#define ACTOR_OPTION_TALK 1
#define ACTOR_OPTION_HEAL 2
#define ACTOR_OPTION_SUPPLIES 3
#define ACTOR_OPTION_MISSION 4

static gActorTarget[MAX_PLAYERS];
static bool:gActorContextActive[MAX_PLAYERS];
static gActorNextRefresh[MAX_PLAYERS];
static Float:gActorX[MAX_PLAYERS];
static Float:gActorY[MAX_PLAYERS];
static Float:gActorZ[MAX_PLAYERS];
static gActorWorld[MAX_PLAYERS];

stock ActorTargetId(playerid)
{
	return ACTOR_DEMO_BASE_ID + playerid;
}

stock ResetActorDemoPlayer(playerid)
{
	gActorTarget[playerid] = INVALID_ACTOR_ID;
	gActorContextActive[playerid] = false;
	gActorNextRefresh[playerid] = 0;
	gActorX[playerid] = 0.0;
	gActorY[playerid] = 0.0;
	gActorZ[playerid] = 0.0;
	gActorWorld[playerid] = 0;
	return 1;
}

stock ClearActorTargetContext(playerid)
{
	if (gActorContextActive[playerid])
	{
		SAMPP_TargetClear(playerid);
		gActorContextActive[playerid] = false;
	}
	return 1;
}

stock DestroyActorTarget(playerid)
{
	if (gActorTarget[playerid] != INVALID_ACTOR_ID)
	{
		DestroyActor(gActorTarget[playerid]);
	}

	gActorTarget[playerid] = INVALID_ACTOR_ID;
	gActorContextActive[playerid] = false;
	return 1;
}

stock bool:HasActorTarget(playerid)
{
	return gActorTarget[playerid] != INVALID_ACTOR_ID
		&& IsValidActor(gActorTarget[playerid]);
}

stock bool:IsLookingAtActorTarget(playerid)
{
	if (!HasActorTarget(playerid))
	{
		return false;
	}

	new bool:cameraTargetMatches = GetPlayerCameraTargetActor(playerid) == gActorTarget[playerid];

	new Float:camX, Float:camY, Float:camZ;
	new Float:frontX, Float:frontY, Float:frontZ;
	if (!GetPlayerCameraPos(playerid, camX, camY, camZ)
		|| !GetPlayerCameraFrontVector(playerid, frontX, frontY, frontZ))
	{
		return false;
	}

	new Float:toX = gActorX[playerid] - camX;
	new Float:toY = gActorY[playerid] - camY;
	new Float:toZ = (gActorZ[playerid] + 0.75) - camZ;
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

stock bool:IsNearActorTarget(playerid)
{
	if (!HasActorTarget(playerid))
	{
		return false;
	}

	if (IsPlayerInAnyVehicle(playerid))
	{
		return false;
	}

	if (GetPlayerVirtualWorld(playerid) != gActorWorld[playerid])
	{
		return false;
	}

	return GetPlayerDistanceFromPoint(playerid, gActorX[playerid], gActorY[playerid], gActorZ[playerid]) <= ACTOR_DEMO_RANGE
		&& IsLookingAtActorTarget(playerid);
}

stock bool:ShouldForceClearActorTarget(playerid)
{
	if (!HasActorTarget(playerid))
	{
		return true;
	}

	if (IsPlayerInAnyVehicle(playerid))
	{
		return true;
	}

	if (GetPlayerVirtualWorld(playerid) != gActorWorld[playerid])
	{
		return true;
	}

	return GetPlayerDistanceFromPoint(playerid, gActorX[playerid], gActorY[playerid], gActorZ[playerid]) > ACTOR_DEMO_CLEAR_RANGE
		|| !IsLookingAtActorTarget(playerid);
}

stock bool:RequireActorTarget(playerid)
{
	if (!HasActorTarget(playerid))
	{
		SendClientMessage(playerid, ACTOR_DEMO_WARN_COLOUR, "[ActorTarget] No actor. Use /targetactor.");
		return false;
	}

	if (!IsNearActorTarget(playerid))
	{
		SendClientMessage(playerid, ACTOR_DEMO_WARN_COLOUR, "[ActorTarget] Move closer to Doctor Maria.");
		return false;
	}

	return true;
}

stock SpawnActorTarget(playerid)
{
	ClearActorTargetContext(playerid);
	DestroyActorTarget(playerid);

	new Float:x, Float:y, Float:z, Float:a;
	if (!GetPlayerPos(playerid, x, y, z))
	{
		SendClientMessage(playerid, ACTOR_DEMO_WARN_COLOUR, "[ActorTarget] Could not read your position.");
		return 1;
	}
	GetPlayerFacingAngle(playerid, a);

	gActorX[playerid] = x + (floatsin(-a, degrees) * 2.2);
	gActorY[playerid] = y + (floatcos(-a, degrees) * 2.2);
	gActorZ[playerid] = z;
	gActorWorld[playerid] = GetPlayerVirtualWorld(playerid);

	gActorTarget[playerid] = CreateActor(ACTOR_DEMO_SKIN, gActorX[playerid], gActorY[playerid], gActorZ[playerid], a + 180.0);
	if (gActorTarget[playerid] == INVALID_ACTOR_ID)
	{
		SendClientMessage(playerid, ACTOR_DEMO_WARN_COLOUR, "[ActorTarget] Could not create actor.");
		return 1;
	}

	SetActorVirtualWorld(gActorTarget[playerid], gActorWorld[playerid]);
	SetActorInvulnerable(gActorTarget[playerid], true);
	ApplyActorAnimation(gActorTarget[playerid], "PED", "IDLE_CHAT", 4.1, true, false, false, false, 0);

	SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "[ActorTarget] Doctor Maria spawned. Move close, look at her, press ALT once.");
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] This actor demo now uses the same standard target menu model as /targetveh.");
	return 1;
}

stock PublishActorTarget(playerid)
{
	if (!IsUsingSAMPP(playerid) || !SAMPP_HasFeature(playerid, SAMPP_FEATURE_TARGET))
	{
		ClearActorTargetContext(playerid);
		return 1;
	}

	if (!IsNearActorTarget(playerid))
	{
		ClearActorTargetContext(playerid);
		return 1;
	}

	new targetid = ActorTargetId(playerid);
	if (!SAMPP_TargetBeginEx(playerid, targetid, SAMPP_TARGET_TYPE_ACTOR, "Doctor Maria", ACTOR_DEMO_TTL_MS))
	{
		ClearActorTargetContext(playerid);
		return 1;
	}

	SAMPP_TargetSetLayout(playerid, targetid, SAMPP_TARGET_LAYOUT_STANDARD);
	SAMPP_TargetSetDescription(playerid, targetid, "Talk to Doctor Maria. Every option is validated by the server before it runs.");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_TALK, "Talk About Work", "talk");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_HEAL, "Request Health Check", "heal");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_SUPPLIES, "Buy Medical Supplies", "shop");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_MISSION, "Ask For A Task", "job");
	SAMPP_TargetCommit(playerid, targetid);

	gActorContextActive[playerid] = true;
	return 1;
}

stock RefreshActorTarget(playerid, bool:force = false)
{
	new now = GetTickCount();
	if (gActorContextActive[playerid] && ShouldForceClearActorTarget(playerid))
	{
		ClearActorTargetContext(playerid);
		gActorNextRefresh[playerid] = now + ACTOR_DEMO_REFRESH_MS;
		return 1;
	}

	if (!force && now < gActorNextRefresh[playerid])
	{
		return 1;
	}

	gActorNextRefresh[playerid] = now + ACTOR_DEMO_REFRESH_MS;
	return PublishActorTarget(playerid);
}

stock RunActorOption(playerid, optionid)
{
	if (!RequireActorTarget(playerid))
	{
		return 1;
	}

	switch (optionid)
	{
		case ACTOR_OPTION_TALK:
		{
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Doctor Maria: I can help with supplies, health checks, or simple field work.");
		}
		case ACTOR_OPTION_HEAL:
		{
			SetPlayerHealth(playerid, 100.0);
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Doctor Maria: You look better now.");
		}
		case ACTOR_OPTION_SUPPLIES:
		{
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Doctor Maria: Medical supplies menu would open here.");
		}
		case ACTOR_OPTION_MISSION:
		{
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Doctor Maria: Bring me three water bottles from the desert camp.");
		}
		default:
		{
			SendClientMessage(playerid, ACTOR_DEMO_WARN_COLOUR, "[ActorTarget] Unknown actor option.");
		}
	}

	ClearActorTargetContext(playerid);
	gActorNextRefresh[playerid] = GetTickCount() + ACTOR_DEMO_REFRESH_MS;
	return 1;
}

stock SendActorTargetHelp(playerid)
{
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] /targetactor spawns Doctor Maria.");
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] Stand close to her and press ALT once to open the standard target menu.");
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] /actorclear removes her. /actorinfo checks Target UI support.");
	return 1;
}

stock SendActorTargetInfo(playerid)
{
	new message[144];
	format(message, sizeof message, "[ActorTarget] IsUsingSAMPP=%d TargetFeature=%d UIFeature=%d actorid=%d", IsUsingSAMPP(playerid), SAMPP_HasFeature(playerid, SAMPP_FEATURE_TARGET), SAMPP_HasFeature(playerid, SAMPP_FEATURE_UI), gActorTarget[playerid]);
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, message);
	return 1;
}

public OnFilterScriptInit()
{
	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		ResetActorDemoPlayer(playerid);
	}

	print("[sampp_actortargetdemo] loaded. Use /targetactor, then press ALT near Doctor Maria.");
	return 1;
}

public OnFilterScriptExit()
{
	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		ClearActorTargetContext(playerid);
		DestroyActorTarget(playerid);
		ResetActorDemoPlayer(playerid);
	}

	print("[sampp_actortargetdemo] unloaded.");
	return 1;
}

public OnPlayerConnect(playerid)
{
	ResetActorDemoPlayer(playerid);
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] Use /targetactor to test actor target options.");
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	ClearActorTargetContext(playerid);
	DestroyActorTarget(playerid);
	ResetActorDemoPlayer(playerid);
	return 1;
}

public OnPlayerUpdate(playerid)
{
	RefreshActorTarget(playerid);
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (!strcmp(cmdtext, "/actorhelp", true) || !strcmp(cmdtext, "/targetactorhelp", true))
	{
		return SendActorTargetHelp(playerid);
	}

	if (!strcmp(cmdtext, "/actorinfo", true))
	{
		return SendActorTargetInfo(playerid);
	}

	if (!strcmp(cmdtext, "/targetactor", true) || !strcmp(cmdtext, "/actortarget", true))
	{
		return SpawnActorTarget(playerid);
	}

	if (!strcmp(cmdtext, "/actorclear", true) || !strcmp(cmdtext, "/targetactorclear", true))
	{
		ClearActorTargetContext(playerid);
		DestroyActorTarget(playerid);
		SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] Actor target cleared.");
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
	if (targetid != ActorTargetId(playerid))
	{
		return 1;
	}

	return RunActorOption(playerid, optionid);
}
