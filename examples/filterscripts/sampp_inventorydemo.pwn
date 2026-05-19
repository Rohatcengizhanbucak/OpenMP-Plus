#include <open.mp>
#include <sampp>

#define INVENTORY_DEMO_COLOUR 0x74D9FFFF
#define INVENTORY_DEMO_OK_COLOUR 0x9DFF86FF
#define INVENTORY_DEMO_WARN_COLOUR 0xFFB86CFF

#define INVENTORY_DEMO_DOC "demo_inventory"

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

stock OpenInventoryDemo(playerid)
{
	if (!RequireInventoryDemo(playerid))
	{
		return 1;
	}

	SAMPP_InventoryOpen(
		playerid,
		INVENTORY_DEMO_DOC,
		"Survival Inventory",
		30,
		"Server-driven inventory. The client only draws and reports clicks; Pawn owns all items."
	);
	SAMPP_InventoryClear(playerid, INVENTORY_DEMO_DOC);

	SAMPP_InventorySetSlot(playerid, INVENTORY_DEMO_DOC, 0, 1001, 4, "Wood", "Basic building material used by the build demo.", "wood");
	SAMPP_InventorySetSlot(playerid, INVENTORY_DEMO_DOC, 1, 1002, 2, "Stone", "Heavier construction material for stronger structures.", "stone");
	SAMPP_InventorySetSlot(playerid, INVENTORY_DEMO_DOC, 2, 2001, 1, "Bandage", "A small medical item. Click events return to Pawn.", "medical");
	SAMPP_InventorySetSlot(playerid, INVENTORY_DEMO_DOC, 6, 3001, 1, "Hammer", "A tool item that could open repair or upgrade actions.", "tool");
	SAMPP_InventorySetSlot(playerid, INVENTORY_DEMO_DOC, 7, 4001, 12, "Ammo", "Stacked item example.", "ammo");

	SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[InventoryDemo] Inventory opened. Hover slots, LMB/RMB sends events to Pawn.");
	return 1;
}

public OnFilterScriptInit()
{
	print("[sampp_inventorydemo] loaded. Use /inventorydemo.");
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
		SAMPP_UIClose(playerid, INVENTORY_DEMO_DOC);
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
	format(
		message,
		sizeof message,
		"[InventoryDemo] event=%d slot=%d item=%s",
		event_type,
		slot,
		payload
	);
	SendClientMessage(playerid, INVENTORY_DEMO_COLOUR, message);
	return 1;
}

public OnPlayerOMPPlusUIEvent(playerid, documentid[], event_type, slot, element[], payload[])
{
	if (!strcmp(documentid, INVENTORY_DEMO_DOC, true) && event_type == SAMPP_UI_EVENT_CLOSE)
	{
		SendClientMessage(playerid, INVENTORY_DEMO_COLOUR, "[InventoryDemo] Inventory closed from the client.");
	}
	return 1;
}
