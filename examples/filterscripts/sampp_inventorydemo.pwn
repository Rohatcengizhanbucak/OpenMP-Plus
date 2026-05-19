#include <open.mp>
#include <sampp>

#define INVENTORY_DEMO_COLOUR 0x74D9FFFF
#define INVENTORY_DEMO_OK_COLOUR 0x9DFF86FF
#define INVENTORY_DEMO_WARN_COLOUR 0xFFB86CFF

#define INVENTORY_DEMO_DOC "demo_inventory"
#define INVENTORY_DEMO_SLOTS 30
#define INVENTORY_DEMO_NAME_LEN 32
#define INVENTORY_DEMO_DESC_LEN 96
#define INVENTORY_DEMO_ICON_LEN 24
#define INVENTORY_DEMO_MAX_STACK 999

#define INV_ITEM_WOOD 1001
#define INV_ITEM_STONE 1002
#define INV_ITEM_BANDAGE 2001
#define INV_ITEM_HAMMER 3001
#define INV_ITEM_AMMO 4001
#define INV_ITEM_WATER_BOTTLE 5001

static bool:gInventoryInitialised[MAX_PLAYERS];
static bool:gInventoryOpen[MAX_PLAYERS];
static gInventoryItem[MAX_PLAYERS][INVENTORY_DEMO_SLOTS];
static gInventoryAmount[MAX_PLAYERS][INVENTORY_DEMO_SLOTS];
static gInventoryName[MAX_PLAYERS][INVENTORY_DEMO_SLOTS][INVENTORY_DEMO_NAME_LEN];
static gInventoryDescription[MAX_PLAYERS][INVENTORY_DEMO_SLOTS][INVENTORY_DEMO_DESC_LEN];
static gInventoryIcon[MAX_PLAYERS][INVENTORY_DEMO_SLOTS][INVENTORY_DEMO_ICON_LEN];

forward SAMPP_InventoryDemoAddItem(playerid, itemid, amount, name[], description[], icon[]);

stock bool:RequireInventoryDemo(playerid)
{
	if (!IsUsingSAMPP(playerid))
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] OpenMP-Plus client is required.");
		return false;
	}

	if (!SAMPP_HasRmlUi(playerid))
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] This client does not report the RmlUi capability.");
		return false;
	}

	return true;
}

stock ClearInventorySlot(playerid, slot)
{
	if (slot < 0 || slot >= INVENTORY_DEMO_SLOTS)
	{
		return 0;
	}

	gInventoryItem[playerid][slot] = 0;
	gInventoryAmount[playerid][slot] = 0;
	gInventoryName[playerid][slot][0] = EOS;
	gInventoryDescription[playerid][slot][0] = EOS;
	gInventoryIcon[playerid][slot][0] = EOS;
	return 1;
}

stock SetInventorySlot(playerid, slot, itemid, amount, const name[], const description[], const icon[])
{
	if (slot < 0 || slot >= INVENTORY_DEMO_SLOTS || itemid <= 0 || amount <= 0)
	{
		return 0;
	}

	gInventoryItem[playerid][slot] = itemid;
	gInventoryAmount[playerid][slot] = amount;
	format(gInventoryName[playerid][slot], INVENTORY_DEMO_NAME_LEN, "%s", name);
	format(gInventoryDescription[playerid][slot], INVENTORY_DEMO_DESC_LEN, "%s", description);
	format(gInventoryIcon[playerid][slot], INVENTORY_DEMO_ICON_LEN, "%s", icon);
	return 1;
}

stock ResetInventoryDemo(playerid)
{
	for (new slot = 0; slot < INVENTORY_DEMO_SLOTS; slot++)
	{
		ClearInventorySlot(playerid, slot);
	}

	SetInventorySlot(playerid, 0, INV_ITEM_WOOD, 4, "Wood", "Basic building material used by the build demo.", "wood");
	SetInventorySlot(playerid, 1, INV_ITEM_STONE, 2, "Stone", "Heavier construction material for stronger structures.", "stone");
	SetInventorySlot(playerid, 2, INV_ITEM_BANDAGE, 1, "Bandage", "A small medical item. Drag events return to Pawn.", "medical");
	SetInventorySlot(playerid, 6, INV_ITEM_HAMMER, 1, "Hammer", "A tool item that could open repair or upgrade actions.", "tool");
	SetInventorySlot(playerid, 7, INV_ITEM_AMMO, 12, "Ammo", "Stacked item example.", "ammo");

	gInventoryInitialised[playerid] = true;
	gInventoryOpen[playerid] = false;
	return 1;
}

stock RefreshInventoryDemo(playerid)
{
	if (!gInventoryOpen[playerid] || !IsUsingSAMPP(playerid))
	{
		return 1;
	}

	SAMPP_InventoryClear(playerid, INVENTORY_DEMO_DOC);

	for (new slot = 0; slot < INVENTORY_DEMO_SLOTS; slot++)
	{
		if (gInventoryItem[playerid][slot] <= 0 || gInventoryAmount[playerid][slot] <= 0)
		{
			continue;
		}

		SAMPP_InventorySetSlot(
			playerid,
			INVENTORY_DEMO_DOC,
			slot,
			gInventoryItem[playerid][slot],
			gInventoryAmount[playerid][slot],
			gInventoryName[playerid][slot],
			gInventoryDescription[playerid][slot],
			gInventoryIcon[playerid][slot]
		);

		new actions[128];
		if (gInventoryAmount[playerid][slot] > 1)
		{
			format(actions, sizeof actions, "use:Use;split:Split Stack;drop:Drop;inspect:Inspect");
		}
		else
		{
			format(actions, sizeof actions, "use:Use;drop:Drop;inspect:Inspect");
		}
		SAMPP_InventorySetSlotActions(playerid, INVENTORY_DEMO_DOC, slot, actions);
	}

	return 1;
}

stock FindFirstEmptyInventorySlot(playerid)
{
	for (new slot = 0; slot < INVENTORY_DEMO_SLOTS; slot++)
	{
		if (gInventoryItem[playerid][slot] == 0)
		{
			return slot;
		}
	}

	return -1;
}

stock FindStackableInventorySlot(playerid, itemid)
{
	for (new slot = 0; slot < INVENTORY_DEMO_SLOTS; slot++)
	{
		if (gInventoryItem[playerid][slot] == itemid && gInventoryAmount[playerid][slot] < INVENTORY_DEMO_MAX_STACK)
		{
			return slot;
		}
	}

	return -1;
}

stock bool:AddInventoryDemoItem(playerid, itemid, amount, const name[], const description[], const icon[])
{
	if (!gInventoryInitialised[playerid])
	{
		ResetInventoryDemo(playerid);
	}

	if (amount <= 0)
	{
		return false;
	}

	new remaining = amount;
	while (remaining > 0)
	{
		new slot = FindStackableInventorySlot(playerid, itemid);
		if (slot == -1)
		{
			break;
		}

		new space = INVENTORY_DEMO_MAX_STACK - gInventoryAmount[playerid][slot];
		new add = remaining < space ? remaining : space;
		gInventoryAmount[playerid][slot] += add;
		remaining -= add;
	}

	while (remaining > 0)
	{
		new slot = FindFirstEmptyInventorySlot(playerid);
		if (slot == -1)
		{
			SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] Inventory is full.");
			RefreshInventoryDemo(playerid);
			return remaining != amount;
		}

		new add = remaining < INVENTORY_DEMO_MAX_STACK ? remaining : INVENTORY_DEMO_MAX_STACK;
		SetInventorySlot(playerid, slot, itemid, add, name, description, icon);
		remaining -= add;
	}

	RefreshInventoryDemo(playerid);
	return true;
}

stock SwapInventorySlots(playerid, from_slot, to_slot)
{
	new item = gInventoryItem[playerid][from_slot];
	new amount = gInventoryAmount[playerid][from_slot];
	new name[INVENTORY_DEMO_NAME_LEN];
	new description[INVENTORY_DEMO_DESC_LEN];
	new icon[INVENTORY_DEMO_ICON_LEN];

	format(name, sizeof name, "%s", gInventoryName[playerid][from_slot]);
	format(description, sizeof description, "%s", gInventoryDescription[playerid][from_slot]);
	format(icon, sizeof icon, "%s", gInventoryIcon[playerid][from_slot]);

	SetInventorySlot(
		playerid,
		from_slot,
		gInventoryItem[playerid][to_slot],
		gInventoryAmount[playerid][to_slot],
		gInventoryName[playerid][to_slot],
		gInventoryDescription[playerid][to_slot],
		gInventoryIcon[playerid][to_slot]
	);

	SetInventorySlot(playerid, to_slot, item, amount, name, description, icon);
	return 1;
}

stock MoveInventorySlot(playerid, from_slot, to_slot)
{
	if (from_slot < 0 || from_slot >= INVENTORY_DEMO_SLOTS || to_slot < 0 || to_slot >= INVENTORY_DEMO_SLOTS || from_slot == to_slot)
	{
		return 0;
	}

	if (gInventoryItem[playerid][from_slot] == 0)
	{
		return 0;
	}

	if (gInventoryItem[playerid][to_slot] == 0)
	{
		SetInventorySlot(
			playerid,
			to_slot,
			gInventoryItem[playerid][from_slot],
			gInventoryAmount[playerid][from_slot],
			gInventoryName[playerid][from_slot],
			gInventoryDescription[playerid][from_slot],
			gInventoryIcon[playerid][from_slot]
		);
		ClearInventorySlot(playerid, from_slot);
		return 1;
	}

	if (gInventoryItem[playerid][from_slot] == gInventoryItem[playerid][to_slot])
	{
		new space = INVENTORY_DEMO_MAX_STACK - gInventoryAmount[playerid][to_slot];
		if (space <= 0)
		{
			return 1;
		}

		if (gInventoryAmount[playerid][from_slot] <= space)
		{
			gInventoryAmount[playerid][to_slot] += gInventoryAmount[playerid][from_slot];
			ClearInventorySlot(playerid, from_slot);
		}
		else
		{
			gInventoryAmount[playerid][to_slot] = INVENTORY_DEMO_MAX_STACK;
			gInventoryAmount[playerid][from_slot] -= space;
		}
		return 1;
	}

	return SwapInventorySlots(playerid, from_slot, to_slot);
}

stock bool:SplitInventoryStack(playerid, slot, amount)
{
	if (slot < 0 || slot >= INVENTORY_DEMO_SLOTS || amount <= 0 || gInventoryItem[playerid][slot] == 0)
	{
		return false;
	}

	if (gInventoryAmount[playerid][slot] <= 1 || amount >= gInventoryAmount[playerid][slot])
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] Split rejected: invalid stack amount.");
		return false;
	}

	new target = FindFirstEmptyInventorySlot(playerid);
	if (target == -1)
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] Split rejected: no empty slot.");
		return false;
	}

	SetInventorySlot(
		playerid,
		target,
		gInventoryItem[playerid][slot],
		amount,
		gInventoryName[playerid][slot],
		gInventoryDescription[playerid][slot],
		gInventoryIcon[playerid][slot]
	);
	gInventoryAmount[playerid][slot] -= amount;

	new message[128];
	format(message, sizeof message, "[InventoryDemo] split %d item(s) from slot %d into slot %d.", amount, slot, target);
	SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, message);
	RefreshInventoryDemo(playerid);
	return true;
}

stock bool:UseInventorySlot(playerid, slot)
{
	if (slot < 0 || slot >= INVENTORY_DEMO_SLOTS || gInventoryItem[playerid][slot] == 0)
	{
		return false;
	}

	switch (gInventoryItem[playerid][slot])
	{
		case INV_ITEM_WATER_BOTTLE:
		{
			SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[InventoryDemo] You drank a Water Bottle.");
			gInventoryAmount[playerid][slot]--;
		}
		case INV_ITEM_BANDAGE:
		{
			new Float:health;
			GetPlayerHealth(playerid, health);
			health += 25.0;
			if (health > 100.0)
			{
				health = 100.0;
			}
			SetPlayerHealth(playerid, health);
			SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[InventoryDemo] Bandage used: health restored.");
			gInventoryAmount[playerid][slot]--;
		}
		case INV_ITEM_HAMMER:
		{
			SendClientMessage(playerid, INVENTORY_DEMO_COLOUR, "[InventoryDemo] Hammer selected. A real mode could open repair/upgrade actions.");
			return true;
		}
		default:
		{
			SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] This item has no use action in the demo.");
			return false;
		}
	}

	if (gInventoryAmount[playerid][slot] <= 0)
	{
		ClearInventorySlot(playerid, slot);
	}
	RefreshInventoryDemo(playerid);
	return true;
}

stock InspectInventorySlot(playerid, slot)
{
	if (slot < 0 || slot >= INVENTORY_DEMO_SLOTS || gInventoryItem[playerid][slot] == 0)
	{
		return 0;
	}

	new message[160];
	format(
		message,
		sizeof message,
		"[InventoryDemo] %s x%d | itemid=%d | %s",
		gInventoryName[playerid][slot],
		gInventoryAmount[playerid][slot],
		gInventoryItem[playerid][slot],
		gInventoryDescription[playerid][slot]
	);
	SendClientMessage(playerid, INVENTORY_DEMO_COLOUR, message);
	return 1;
}

stock bool:DropInventorySlotToWorld(playerid, slot)
{
	if (slot < 0 || slot >= INVENTORY_DEMO_SLOTS || gInventoryItem[playerid][slot] == 0)
	{
		return false;
	}

	if (gInventoryItem[playerid][slot] != INV_ITEM_WATER_BOTTLE)
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] This demo only world-drops Water Bottle items.");
		return false;
	}

	new result = CallRemoteFunction("SAMPP_ItemDemoSpawnWaterBottleForPlayer", "i", playerid);
	if (!result)
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] sampp_itemdemo is not loaded, so the bottle cannot be dropped to the world.");
		return false;
	}

	gInventoryAmount[playerid][slot]--;
	if (gInventoryAmount[playerid][slot] <= 0)
	{
		ClearInventorySlot(playerid, slot);
	}

	RefreshInventoryDemo(playerid);
	SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[InventoryDemo] Water Bottle dropped back into the world.");
	return true;
}

stock OpenInventoryDemo(playerid)
{
	if (!RequireInventoryDemo(playerid))
	{
		return 1;
	}

	if (!gInventoryInitialised[playerid])
	{
		ResetInventoryDemo(playerid);
	}

	SAMPP_InventoryOpen(
		playerid,
		INVENTORY_DEMO_DOC,
		"Survival Inventory",
		INVENTORY_DEMO_SLOTS,
		"Server-driven inventory. Drag slots to move, merge, or swap. Drag outside the panel to drop supported items into the world."
	);

	gInventoryOpen[playerid] = true;
	RefreshInventoryDemo(playerid);

	SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[InventoryDemo] Inventory opened. Drag slots; drop outside to world-drop supported items.");
	return 1;
}

public SAMPP_InventoryDemoAddItem(playerid, itemid, amount, name[], description[], icon[])
{
	return AddInventoryDemoItem(playerid, itemid, amount, name, description, icon) ? 1 : 0;
}

public OnFilterScriptInit()
{
	print("[sampp_inventorydemo] loaded. Use /inventorydemo. /itemadd pickups now feed this inventory.");
	return 1;
}

public OnPlayerConnect(playerid)
{
	ResetInventoryDemo(playerid);
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	ResetInventoryDemo(playerid);
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (!strcmp(cmdtext, "/inventorydemo", true) || !strcmp(cmdtext, "/invdemo", true))
	{
		return OpenInventoryDemo(playerid);
	}

	if (!strcmp(cmdtext, "/inventoryclose", true) || !strcmp(cmdtext, "/invclose", true))
	{
		gInventoryOpen[playerid] = false;
		SAMPP_UIClose(playerid, INVENTORY_DEMO_DOC);
		return 1;
	}

	if (!strcmp(cmdtext, "/inventoryreset", true) || !strcmp(cmdtext, "/invreset", true))
	{
		ResetInventoryDemo(playerid);
		gInventoryOpen[playerid] = true;
		OpenInventoryDemo(playerid);
		SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[InventoryDemo] Demo inventory reset.");
		return 1;
	}

	return 0;
}

public OnPlayerOMPPlusInventoryClick(playerid, documentid[], slot, event_type, payload[])
{
	if (strcmp(documentid, INVENTORY_DEMO_DOC, true))
	{
		return 1;
	}

	new message[144];
	format(message, sizeof message, "[InventoryDemo] click event=%d slot=%d item=%s", event_type, slot, payload);
	SendClientMessage(playerid, INVENTORY_DEMO_COLOUR, message);
	return 1;
}

public OnPlayerOMPPlusInventoryDrop(playerid, documentid[], from_slot, to_slot, payload[])
{
	if (strcmp(documentid, INVENTORY_DEMO_DOC, true))
	{
		return 1;
	}

	if (from_slot < 0 || from_slot >= INVENTORY_DEMO_SLOTS || gInventoryItem[playerid][from_slot] == 0)
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] Drop rejected: source slot is empty or invalid.");
		RefreshInventoryDemo(playerid);
		return 1;
	}

	if (to_slot == -1)
	{
		DropInventorySlotToWorld(playerid, from_slot);
		return 1;
	}

	if (MoveInventorySlot(playerid, from_slot, to_slot))
	{
		new message[128];
		format(message, sizeof message, "[InventoryDemo] moved slot %d -> %d.", from_slot, to_slot);
		SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, message);
		RefreshInventoryDemo(playerid);
	}
	else
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] Move rejected.");
		RefreshInventoryDemo(playerid);
	}

	return 1;
}

public OnPlayerOMPPlusInventoryAction(playerid, documentid[], slot, action[], payload[])
{
	if (strcmp(documentid, INVENTORY_DEMO_DOC, true))
	{
		return 1;
	}

	if (!strcmp(action, "use", true))
	{
		UseInventorySlot(playerid, slot);
	}
	else if (!strcmp(action, "drop", true))
	{
		DropInventorySlotToWorld(playerid, slot);
	}
	else if (!strcmp(action, "inspect", true))
	{
		InspectInventorySlot(playerid, slot);
	}
	else
	{
		new message[128];
		format(message, sizeof message, "[InventoryDemo] unknown action '%s' for slot %d.", action, slot);
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, message);
	}

	return 1;
}

public OnPlayerOMPPlusInventorySplit(playerid, documentid[], slot, amount, payload[])
{
	if (strcmp(documentid, INVENTORY_DEMO_DOC, true))
	{
		return 1;
	}

	SplitInventoryStack(playerid, slot, amount);
	return 1;
}

public OnPlayerOMPPlusUIEvent(playerid, documentid[], event_type, slot, element[], payload[])
{
	if (!strcmp(documentid, INVENTORY_DEMO_DOC, true) && event_type == SAMPP_UI_EVENT_CLOSE)
	{
		gInventoryOpen[playerid] = false;
		SendClientMessage(playerid, INVENTORY_DEMO_COLOUR, "[InventoryDemo] Inventory closed from the client.");
	}
	return 1;
}
