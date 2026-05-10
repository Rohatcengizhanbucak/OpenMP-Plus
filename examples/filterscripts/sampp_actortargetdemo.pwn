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

#define ACTOR_TYPE_DOCTOR 0
#define ACTOR_TYPE_MECHANIC 1
#define ACTOR_TYPE_TRADER 2
#define ACTOR_TYPE_GUARD 3

#define ACTOR_OPTION_TALK 1
#define ACTOR_OPTION_HEAL 2
#define ACTOR_OPTION_SUPPLIES 3
#define ACTOR_OPTION_MISSION 4
#define ACTOR_OPTION_REPAIR 5
#define ACTOR_OPTION_PARTS 6
#define ACTOR_OPTION_TRADE 7
#define ACTOR_OPTION_DELIVERY 8
#define ACTOR_OPTION_ACCESS 9
#define ACTOR_OPTION_REPORT 10

static gActorTarget[MAX_PLAYERS];
static gActorType[MAX_PLAYERS];
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
	gActorType[playerid] = ACTOR_TYPE_DOCTOR;
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

stock ActorTypeSkin(actorType)
{
	switch (actorType)
	{
		case ACTOR_TYPE_MECHANIC:
		{
			return 50;
		}
		case ACTOR_TYPE_TRADER:
		{
			return 29;
		}
		case ACTOR_TYPE_GUARD:
		{
			return 71;
		}
	}
	return ACTOR_DEMO_SKIN;
}

stock GetActorTypeTitle(actorType, title[], size = sizeof title)
{
	switch (actorType)
	{
		case ACTOR_TYPE_MECHANIC:
		{
			format(title, size, "Garage Mechanic");
		}
		case ACTOR_TYPE_TRADER:
		{
			format(title, size, "Desert Trader");
		}
		case ACTOR_TYPE_GUARD:
		{
			format(title, size, "Security Guard");
		}
		default:
		{
			format(title, size, "Doctor Maria");
		}
	}
	return 1;
}

stock GetActorTypeDescription(actorType, description[], size = sizeof description)
{
	switch (actorType)
	{
		case ACTOR_TYPE_MECHANIC:
		{
			format(description, size, "Vehicle support, repair advice, and parts requests are grouped by category.");
		}
		case ACTOR_TYPE_TRADER:
		{
			format(description, size, "Trade, supplies, and delivery work are separated into clear interaction groups.");
		}
		case ACTOR_TYPE_GUARD:
		{
			format(description, size, "Access control and incident reporting actions are validated by the server.");
		}
		default:
		{
			format(description, size, "Talk to Doctor Maria. Every option is validated by the server before it runs.");
		}
	}
	return 1;
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

stock SpawnActorTarget(playerid, actorType = ACTOR_TYPE_DOCTOR)
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
	gActorType[playerid] = actorType;

	gActorTarget[playerid] = CreateActor(ActorTypeSkin(actorType), gActorX[playerid], gActorY[playerid], gActorZ[playerid], a + 180.0);
	if (gActorTarget[playerid] == INVALID_ACTOR_ID)
	{
		SendClientMessage(playerid, ACTOR_DEMO_WARN_COLOUR, "[ActorTarget] Could not create actor.");
		return 1;
	}

	SetActorVirtualWorld(gActorTarget[playerid], gActorWorld[playerid]);
	SetActorInvulnerable(gActorTarget[playerid], true);
	ApplyActorAnimation(gActorTarget[playerid], "PED", "IDLE_CHAT", 4.1, true, false, false, false, 0);

	new title[48], message[128];
	GetActorTypeTitle(actorType, title);
	format(message, sizeof message, "[ActorTarget] %s spawned. Move close, look at the actor, press ALT once.", title);
	SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, message);
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] Category rows are Pawn-driven; the stable vehicle-style menu renderer is unchanged.");
	return 1;
}

stock AddDoctorActorMenu(playerid, targetid)
{
	SAMPP_TargetAddHeader(playerid, targetid, "Medical");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_HEAL, "Request Health Check", "heal");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_SUPPLIES, "Buy Medical Supplies", "shop");
	SAMPP_TargetAddDivider(playerid, targetid);
	SAMPP_TargetAddHeader(playerid, targetid, "Work");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_TALK, "Talk About Work", "talk");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_MISSION, "Ask For A Task", "job");
	return 1;
}

stock AddMechanicActorMenu(playerid, targetid)
{
	SAMPP_TargetAddHeader(playerid, targetid, "Vehicle Service");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_REPAIR, "Request Inspection", "repair");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_PARTS, "Ask For Parts", "parts");
	SAMPP_TargetAddDivider(playerid, targetid);
	SAMPP_TargetAddHeader(playerid, targetid, "Garage Work");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_TALK, "Talk About Jobs", "talk");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_MISSION, "Take Tow Task", "task");
	return 1;
}

stock AddTraderActorMenu(playerid, targetid)
{
	SAMPP_TargetAddHeader(playerid, targetid, "Trading");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_TRADE, "Browse Goods", "trade");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_SUPPLIES, "Buy Supplies", "shop");
	SAMPP_TargetAddDivider(playerid, targetid);
	SAMPP_TargetAddHeader(playerid, targetid, "Work");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_DELIVERY, "Ask For Delivery", "route");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_TALK, "Talk About Prices", "talk");
	return 1;
}

stock AddGuardActorMenu(playerid, targetid)
{
	SAMPP_TargetAddHeader(playerid, targetid, "Access");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_ACCESS, "Request Entry", "access");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_TALK, "Ask About Rules", "talk");
	SAMPP_TargetAddDivider(playerid, targetid);
	SAMPP_TargetAddHeader(playerid, targetid, "Security");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_REPORT, "Report Incident", "report");
	SAMPP_TargetAddAction(playerid, targetid, ACTOR_OPTION_MISSION, "Ask For Patrol Task", "job");
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
	new title[48], description[128];
	GetActorTypeTitle(gActorType[playerid], title);
	GetActorTypeDescription(gActorType[playerid], description);

	if (!SAMPP_TargetBeginEx(playerid, targetid, SAMPP_TARGET_TYPE_ACTOR, title, ACTOR_DEMO_TTL_MS))
	{
		ClearActorTargetContext(playerid);
		return 1;
	}

	SAMPP_TargetSetLayout(playerid, targetid, SAMPP_TARGET_LAYOUT_STANDARD);
	SAMPP_TargetSetDescription(playerid, targetid, description);

	switch (gActorType[playerid])
	{
		case ACTOR_TYPE_MECHANIC:
		{
			AddMechanicActorMenu(playerid, targetid);
		}
		case ACTOR_TYPE_TRADER:
		{
			AddTraderActorMenu(playerid, targetid);
		}
		case ACTOR_TYPE_GUARD:
		{
			AddGuardActorMenu(playerid, targetid);
		}
		default:
		{
			AddDoctorActorMenu(playerid, targetid);
		}
	}

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
			switch (gActorType[playerid])
			{
				case ACTOR_TYPE_MECHANIC:
				{
					SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Mechanic: I can inspect vehicles, find parts, or assign tow work.");
				}
				case ACTOR_TYPE_TRADER:
				{
					SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Trader: Prices change by route safety and stock level.");
				}
				case ACTOR_TYPE_GUARD:
				{
					SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Guard: Follow access rules and keep weapons holstered.");
				}
				default:
				{
					SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Doctor Maria: I can help with supplies, health checks, or simple field work.");
				}
			}
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
			switch (gActorType[playerid])
			{
				case ACTOR_TYPE_MECHANIC:
				{
					SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Mechanic: Tow a broken vehicle back to the garage.");
				}
				case ACTOR_TYPE_GUARD:
				{
					SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Guard: Patrol the gate and report suspicious vehicles.");
				}
				default:
				{
					SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Doctor Maria: Bring me three water bottles from the desert camp.");
				}
			}
		}
		case ACTOR_OPTION_REPAIR:
		{
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Mechanic: Vehicle inspection request accepted.");
		}
		case ACTOR_OPTION_PARTS:
		{
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Mechanic: Parts request menu would open here.");
		}
		case ACTOR_OPTION_TRADE:
		{
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Trader: Goods list would open here.");
		}
		case ACTOR_OPTION_DELIVERY:
		{
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Trader: Delivery route offer would be generated here.");
		}
		case ACTOR_OPTION_ACCESS:
		{
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Guard: Access request sent for server-side validation.");
		}
		case ACTOR_OPTION_REPORT:
		{
			SendClientMessage(playerid, ACTOR_DEMO_OK_COLOUR, "Guard: Incident report flow would open here.");
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
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] /targetactor or /targetdoctor spawns Doctor Maria.");
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] Extra types: /targetmechanic /targettrader /targetguard.");
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] Stand close to her and press ALT once to open the standard target menu.");
	SendClientMessage(playerid, ACTOR_DEMO_COLOUR, "[ActorTarget] /actorclear removes her. /actorinfo checks Target UI support.");
	return 1;
}

stock SendActorTargetInfo(playerid)
{
	new message[144];
	format(message, sizeof message, "[ActorTarget] IsUsingSAMPP=%d TargetFeature=%d UIFeature=%d actorid=%d type=%d", IsUsingSAMPP(playerid), SAMPP_HasFeature(playerid, SAMPP_FEATURE_TARGET), SAMPP_HasFeature(playerid, SAMPP_FEATURE_UI), gActorTarget[playerid], gActorType[playerid]);
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

	if (!strcmp(cmdtext, "/targetactor", true) || !strcmp(cmdtext, "/actortarget", true) || !strcmp(cmdtext, "/targetdoctor", true))
	{
		return SpawnActorTarget(playerid, ACTOR_TYPE_DOCTOR);
	}

	if (!strcmp(cmdtext, "/targetmechanic", true))
	{
		return SpawnActorTarget(playerid, ACTOR_TYPE_MECHANIC);
	}

	if (!strcmp(cmdtext, "/targettrader", true))
	{
		return SpawnActorTarget(playerid, ACTOR_TYPE_TRADER);
	}

	if (!strcmp(cmdtext, "/targetguard", true))
	{
		return SpawnActorTarget(playerid, ACTOR_TYPE_GUARD);
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
