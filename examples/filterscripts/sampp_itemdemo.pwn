#include <open.mp>
#include <sampp>

#define ITEM_DEMO_MAX_ITEMS 64
#define ITEM_DEMO_MODEL_WATER_BOTTLE 19570
#define ITEM_DEMO_PICKUP_RANGE 2.0
#define ITEM_DEMO_CAPTURE_TTL_MS 450
#define ITEM_DEMO_CAPTURE_REFRESH_MS 150
#define ITEM_DEMO_OBJECT_DRAW_DISTANCE 50.0
#define ITEM_DEMO_LABEL_DRAW_DISTANCE 16.0

#define ITEM_DEMO_COLOUR 0x74D9FFFF
#define ITEM_DEMO_WARN_COLOUR 0xFFB86CFF

static bool:gItemActive[ITEM_DEMO_MAX_ITEMS];
static gItemObject[ITEM_DEMO_MAX_ITEMS];
static Text3D:gItemLabel[ITEM_DEMO_MAX_ITEMS];
static Float:gItemX[ITEM_DEMO_MAX_ITEMS];
static Float:gItemY[ITEM_DEMO_MAX_ITEMS];
static Float:gItemZ[ITEM_DEMO_MAX_ITEMS];
static gItemWorld[ITEM_DEMO_MAX_ITEMS];
static gItemInterior[ITEM_DEMO_MAX_ITEMS];
static bool:gItemCaptureActive[MAX_PLAYERS];
static gItemNextCaptureRefresh[MAX_PLAYERS];

stock ResetItemSlot(slot)
{
	gItemActive[slot] = false;
	gItemObject[slot] = INVALID_OBJECT_ID;
	gItemLabel[slot] = INVALID_3DTEXT_ID;
	gItemX[slot] = 0.0;
	gItemY[slot] = 0.0;
	gItemZ[slot] = 0.0;
	gItemWorld[slot] = 0;
	gItemInterior[slot] = 0;
}

stock InitItemSlots()
{
	for (new slot = 0; slot < ITEM_DEMO_MAX_ITEMS; slot++)
	{
		ResetItemSlot(slot);
	}
}

stock DestroyItemSlot(slot)
{
	if (!gItemActive[slot])
	{
		return 0;
	}

	if (gItemObject[slot] != INVALID_OBJECT_ID)
	{
		DestroyObject(gItemObject[slot]);
	}

	if (gItemLabel[slot] != INVALID_3DTEXT_ID)
	{
		Delete3DTextLabel(gItemLabel[slot]);
	}

	ResetItemSlot(slot);
	return 1;
}

stock DestroyAllItems()
{
	for (new slot = 0; slot < ITEM_DEMO_MAX_ITEMS; slot++)
	{
		DestroyItemSlot(slot);
	}
}

stock FindFreeItemSlot()
{
	for (new slot = 0; slot < ITEM_DEMO_MAX_ITEMS; slot++)
	{
		if (!gItemActive[slot])
		{
			return slot;
		}
	}

	return -1;
}

stock ResetPlayerItemCapture(playerid)
{
	gItemCaptureActive[playerid] = false;
	gItemNextCaptureRefresh[playerid] = 0;
	return 1;
}

stock PrepareItemKey(playerid)
{
	if (!IsUsingSAMPP(playerid))
	{
		SendClientMessage(playerid, ITEM_DEMO_WARN_COLOUR, "[ItemDemo] SA-MP+ client plugin not detected yet.");
		return 1;
	}

	SAMPP_UnbindKey(playerid, SAMPP_KEY_E);
	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_E, "item_pickup");
	gItemCaptureActive[playerid] = false;

	SendClientMessage(playerid, ITEM_DEMO_COLOUR, "[ItemDemo] E capture activates only near a Water Bottle. Use /itemadd, then press E near it.");
	return 1;
}

stock AddWaterBottleItem(playerid)
{
	new slot = FindFreeItemSlot();
	if (slot == -1)
	{
		SendClientMessage(playerid, ITEM_DEMO_WARN_COLOUR, "[ItemDemo] Item pool is full. Use /itemclear first.");
		return 1;
	}

	new Float:x, Float:y, Float:z;
	if (!GetPlayerPos(playerid, x, y, z))
	{
		SendClientMessage(playerid, ITEM_DEMO_WARN_COLOUR, "[ItemDemo] Could not read your position.");
		return 1;
	}

	gItemX[slot] = x;
	gItemY[slot] = y;
	gItemZ[slot] = z - 0.85;
	gItemWorld[slot] = GetPlayerVirtualWorld(playerid);
	gItemInterior[slot] = GetPlayerInterior(playerid);

	gItemObject[slot] = CreateObject(ITEM_DEMO_MODEL_WATER_BOTTLE, gItemX[slot], gItemY[slot], gItemZ[slot], 0.0, 0.0, 0.0, ITEM_DEMO_OBJECT_DRAW_DISTANCE);
	if (gItemObject[slot] == INVALID_OBJECT_ID)
	{
		ResetItemSlot(slot);
		SendClientMessage(playerid, ITEM_DEMO_WARN_COLOUR, "[ItemDemo] Could not create the item object.");
		return 1;
	}

	gItemLabel[slot] = Create3DTextLabel("{74D9FF}Water Bottle\n{FFFFFF}Press E to pick up", ITEM_DEMO_COLOUR, x, y, z + 0.25, ITEM_DEMO_LABEL_DRAW_DISTANCE, gItemWorld[slot], false);
	if (gItemLabel[slot] == INVALID_3DTEXT_ID)
	{
		DestroyObject(gItemObject[slot]);
		ResetItemSlot(slot);
		SendClientMessage(playerid, ITEM_DEMO_WARN_COLOUR, "[ItemDemo] Could not create the item label.");
		return 1;
	}

	gItemActive[slot] = true;

	new message[128];
	format(message, sizeof message, "[ItemDemo] Water Bottle item #%d added. Press E near it to test the ASI keybind.", slot);
	SendClientMessage(playerid, ITEM_DEMO_COLOUR, message);
	PrepareItemKey(playerid);
	return 1;
}

stock FindNearestItemForPlayer(playerid)
{
	new world = GetPlayerVirtualWorld(playerid);
	new interior = GetPlayerInterior(playerid);
	new nearest = -1;
	new Float:nearestDistance = ITEM_DEMO_PICKUP_RANGE + 1.0;

	for (new slot = 0; slot < ITEM_DEMO_MAX_ITEMS; slot++)
	{
		if (!gItemActive[slot] || gItemWorld[slot] != world || gItemInterior[slot] != interior)
		{
			continue;
		}

		new Float:distance = GetPlayerDistanceFromPoint(playerid, gItemX[slot], gItemY[slot], gItemZ[slot]);
		if (distance <= ITEM_DEMO_PICKUP_RANGE && distance < nearestDistance)
		{
			nearest = slot;
			nearestDistance = distance;
		}
	}

	return nearest;
}

stock PickupNearestItem(playerid)
{
	if (!IsUsingSAMPP(playerid))
	{
		SendClientMessage(playerid, ITEM_DEMO_WARN_COLOUR, "[ItemDemo] SA-MP+ client plugin is required for the E key test.");
		return 1;
	}

	new slot = FindNearestItemForPlayer(playerid);
	if (slot == -1)
	{
		SendClientMessage(playerid, ITEM_DEMO_WARN_COLOUR, "[ItemDemo] No Water Bottle nearby. Use /itemadd or move closer.");
		return 1;
	}

	DestroyItemSlot(slot);
	SAMPP_EndKeyCapture(playerid, SAMPP_KEY_E, "item_pickup");
	gItemCaptureActive[playerid] = false;
	ApplyAnimation(playerid, "BOMBER", "BOM_Plant", 4.0, false, false, false, false, 0);
	SendClientMessage(playerid, ITEM_DEMO_COLOUR, "[ItemDemo] You picked up a Water Bottle through the SA-MP+ E keybind.");
	return 1;
}

stock RefreshItemCapture(playerid, bool:force = false)
{
	if (!IsUsingSAMPP(playerid))
	{
		ResetPlayerItemCapture(playerid);
		return 1;
	}

	new now = GetTickCount();
	if (!force && now < gItemNextCaptureRefresh[playerid])
	{
		return 1;
	}
	gItemNextCaptureRefresh[playerid] = now + ITEM_DEMO_CAPTURE_REFRESH_MS;

	new slot = FindNearestItemForPlayer(playerid);
	if (slot != -1)
	{
		SAMPP_BeginKeyCapture(
			playerid,
			SAMPP_KEY_E,
			SAMPP_KEY_EVENT_DOWN,
			SAMPP_CAPTURE_PRIORITY_ITEM,
			ITEM_DEMO_CAPTURE_TTL_MS,
			SAMPP_CAPTURE_DEFAULT_FLAGS,
			"item_pickup"
		);
		gItemCaptureActive[playerid] = true;
	}
	else if (gItemCaptureActive[playerid])
	{
		SAMPP_EndKeyCapture(playerid, SAMPP_KEY_E, "item_pickup");
		gItemCaptureActive[playerid] = false;
	}

	return 1;
}

stock SendItemHelp(playerid)
{
	SendClientMessage(playerid, ITEM_DEMO_COLOUR, "[ItemDemo] /itemadd creates a Water Bottle at your position.");
	SendClientMessage(playerid, ITEM_DEMO_COLOUR, "[ItemDemo] Press E near the label to pick it up through OnPlayerSAMPPKey.");
	SendClientMessage(playerid, ITEM_DEMO_COLOUR, "[ItemDemo] /itemkeys rebinds E, /itemclear removes all demo items.");
	return 1;
}

public OnFilterScriptInit()
{
	InitItemSlots();
	print("[sampp_itemdemo] loaded. Use /itemadd, then press E near the Water Bottle.");
	return 1;
}

public OnFilterScriptExit()
{
	DestroyAllItems();
	print("[sampp_itemdemo] unloaded.");
	return 1;
}

public OnPlayerConnect(playerid)
{
	ResetPlayerItemCapture(playerid);
	SendClientMessage(playerid, ITEM_DEMO_COLOUR, "[ItemDemo] Use /itemadd to create a Water Bottle pickup demo.");
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	ResetPlayerItemCapture(playerid);
	return 1;
}

public OnPlayerUpdate(playerid)
{
	RefreshItemCapture(playerid);
	return 1;
}

public OnPlayerSAMPPJoin(playerid, bool:has_plugin)
{
	if (has_plugin)
	{
		PrepareItemKey(playerid);
	}
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (!strcmp(cmdtext, "/itemadd", true))
	{
		return AddWaterBottleItem(playerid);
	}

	if (!strcmp(cmdtext, "/itemkeys", true))
	{
		return PrepareItemKey(playerid);
	}

	if (!strcmp(cmdtext, "/itempickup", true))
	{
		return PickupNearestItem(playerid);
	}

	if (!strcmp(cmdtext, "/itemhelp", true))
	{
		return SendItemHelp(playerid);
	}

	if (!strcmp(cmdtext, "/itemclear", true))
	{
		DestroyAllItems();
		SendClientMessage(playerid, ITEM_DEMO_COLOUR, "[ItemDemo] All demo items were removed.");
		return 1;
	}

	return 0;
}

public OnPlayerSAMPPKey(playerid, keyid, keystate, action[])
{
	if (keyid != SAMPP_KEY_E || keystate != SAMPP_KEY_STATE_DOWN || strcmp(action, "item_pickup", true))
	{
		return 1;
	}

	SendClientMessage(playerid, ITEM_DEMO_COLOUR, "[ItemDemo] E key callback received from SA-MP+ ASI.");
	return PickupNearestItem(playerid);
}
