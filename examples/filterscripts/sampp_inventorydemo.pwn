#include <open.mp>
#include <sampp>

#define INVENTORY_DEMO_COLOUR 0x74D9FFFF
#define INVENTORY_DEMO_OK_COLOUR 0x9DFF86FF
#define INVENTORY_DEMO_WARN_COLOUR 0xFFB86CFF

#define INVENTORY_DEMO_DOC "demo_inventory"
#define INVENTORY_WORKSPACE_DOC "demo_workspace"
#define STORAGE_WORKSPACE_DOC "demo_storage"
#define CRAFT_WORKSPACE_DOC "demo_craft"
#define PANE_INVENTORY "inventory"
#define PANE_EQUIPMENT "equipment"
#define PANE_CHEST "chest"
#define PANE_RECIPES "recipes"
#define PANE_QUEUE "queue"
#define INVENTORY_DEMO_SLOTS 30
#define EQUIPMENT_DEMO_SLOTS 10
#define CHEST_DEMO_SLOTS 24
#define RECIPE_DEMO_SLOTS 6
#define QUEUE_DEMO_SLOTS 5
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
#define INV_ITEM_ARMOR 6001
#define INV_ITEM_BACKPACK 6002
#define INV_ITEM_FOUNDATION_KIT 7001
#define INV_ITEM_HATCHET 7002

static bool:gInventoryInitialised[MAX_PLAYERS];
static bool:gInventoryOpen[MAX_PLAYERS];
static bool:gWorkspaceInventoryOpen[MAX_PLAYERS];
static bool:gStorageOpen[MAX_PLAYERS];
static bool:gCraftOpen[MAX_PLAYERS];
static gInventoryItem[MAX_PLAYERS][INVENTORY_DEMO_SLOTS];
static gInventoryAmount[MAX_PLAYERS][INVENTORY_DEMO_SLOTS];
static gInventoryName[MAX_PLAYERS][INVENTORY_DEMO_SLOTS][INVENTORY_DEMO_NAME_LEN];
static gInventoryDescription[MAX_PLAYERS][INVENTORY_DEMO_SLOTS][INVENTORY_DEMO_DESC_LEN];
static gInventoryIcon[MAX_PLAYERS][INVENTORY_DEMO_SLOTS][INVENTORY_DEMO_ICON_LEN];
static gEquipmentItem[MAX_PLAYERS][EQUIPMENT_DEMO_SLOTS];
static gEquipmentAmount[MAX_PLAYERS][EQUIPMENT_DEMO_SLOTS];
static gEquipmentName[MAX_PLAYERS][EQUIPMENT_DEMO_SLOTS][INVENTORY_DEMO_NAME_LEN];
static gEquipmentDescription[MAX_PLAYERS][EQUIPMENT_DEMO_SLOTS][INVENTORY_DEMO_DESC_LEN];
static gEquipmentIcon[MAX_PLAYERS][EQUIPMENT_DEMO_SLOTS][INVENTORY_DEMO_ICON_LEN];
static gChestItem[MAX_PLAYERS][CHEST_DEMO_SLOTS];
static gChestAmount[MAX_PLAYERS][CHEST_DEMO_SLOTS];
static gChestName[MAX_PLAYERS][CHEST_DEMO_SLOTS][INVENTORY_DEMO_NAME_LEN];
static gChestDescription[MAX_PLAYERS][CHEST_DEMO_SLOTS][INVENTORY_DEMO_DESC_LEN];
static gChestIcon[MAX_PLAYERS][CHEST_DEMO_SLOTS][INVENTORY_DEMO_ICON_LEN];

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

stock ClearEquipmentSlot(playerid, slot)
{
	if (slot < 0 || slot >= EQUIPMENT_DEMO_SLOTS)
	{
		return 0;
	}
	gEquipmentItem[playerid][slot] = 0;
	gEquipmentAmount[playerid][slot] = 0;
	gEquipmentName[playerid][slot][0] = EOS;
	gEquipmentDescription[playerid][slot][0] = EOS;
	gEquipmentIcon[playerid][slot][0] = EOS;
	return 1;
}

stock SetEquipmentSlot(playerid, slot, itemid, amount, const name[], const description[], const icon[])
{
	if (slot < 0 || slot >= EQUIPMENT_DEMO_SLOTS || itemid <= 0 || amount <= 0)
	{
		return 0;
	}
	gEquipmentItem[playerid][slot] = itemid;
	gEquipmentAmount[playerid][slot] = amount;
	format(gEquipmentName[playerid][slot], INVENTORY_DEMO_NAME_LEN, "%s", name);
	format(gEquipmentDescription[playerid][slot], INVENTORY_DEMO_DESC_LEN, "%s", description);
	format(gEquipmentIcon[playerid][slot], INVENTORY_DEMO_ICON_LEN, "%s", icon);
	return 1;
}

stock ClearChestSlot(playerid, slot)
{
	if (slot < 0 || slot >= CHEST_DEMO_SLOTS)
	{
		return 0;
	}
	gChestItem[playerid][slot] = 0;
	gChestAmount[playerid][slot] = 0;
	gChestName[playerid][slot][0] = EOS;
	gChestDescription[playerid][slot][0] = EOS;
	gChestIcon[playerid][slot][0] = EOS;
	return 1;
}

stock SetChestSlot(playerid, slot, itemid, amount, const name[], const description[], const icon[])
{
	if (slot < 0 || slot >= CHEST_DEMO_SLOTS || itemid <= 0 || amount <= 0)
	{
		return 0;
	}
	gChestItem[playerid][slot] = itemid;
	gChestAmount[playerid][slot] = amount;
	format(gChestName[playerid][slot], INVENTORY_DEMO_NAME_LEN, "%s", name);
	format(gChestDescription[playerid][slot], INVENTORY_DEMO_DESC_LEN, "%s", description);
	format(gChestIcon[playerid][slot], INVENTORY_DEMO_ICON_LEN, "%s", icon);
	return 1;
}

stock bool:IsInventoryItemUsable(itemid)
{
	switch (itemid)
	{
		case INV_ITEM_WATER_BOTTLE, INV_ITEM_BANDAGE, INV_ITEM_HAMMER:
		{
			return true;
		}
	}
	return false;
}

stock BuildInventoryDemoActions(playerid, slot, actions[], max_len)
{
	if (slot < 0 || slot >= INVENTORY_DEMO_SLOTS || gInventoryItem[playerid][slot] <= 0)
	{
		actions[0] = EOS;
		return 0;
	}

	new bool:usable = IsInventoryItemUsable(gInventoryItem[playerid][slot]);
	new bool:stacked = gInventoryAmount[playerid][slot] > 1;
	if (usable && stacked)
	{
		format(actions, max_len, "use:Use;split:Split Stack;drop:Drop;inspect:Inspect");
	}
	else if (usable)
	{
		format(actions, max_len, "use:Use;drop:Drop;inspect:Inspect");
	}
	else if (stacked)
	{
		format(actions, max_len, "inspect:Inspect;split:Split Stack;drop:Drop");
	}
	else
	{
		format(actions, max_len, "inspect:Inspect;drop:Drop");
	}
	return 1;
}

stock BuildChestDemoActions(playerid, slot, actions[], max_len)
{
	if (slot < 0 || slot >= CHEST_DEMO_SLOTS || gChestItem[playerid][slot] <= 0)
	{
		actions[0] = EOS;
		return 0;
	}

	if (gChestAmount[playerid][slot] > 1)
	{
		format(actions, max_len, "take:Take;split:Split Stack;inspect:Inspect");
	}
	else
	{
		format(actions, max_len, "take:Take;inspect:Inspect");
	}
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
	SetInventorySlot(playerid, 8, INV_ITEM_ARMOR, 1, "Kevlar Vest", "Equipment demo item. Drag to the Armor slot.", "armor");
	SetInventorySlot(playerid, 9, INV_ITEM_BACKPACK, 1, "Field Backpack", "Equipment demo item. Drag to the Backpack slot.", "bag");

	for (new equip = 0; equip < EQUIPMENT_DEMO_SLOTS; equip++)
	{
		ClearEquipmentSlot(playerid, equip);
	}

	for (new chest = 0; chest < CHEST_DEMO_SLOTS; chest++)
	{
		ClearChestSlot(playerid, chest);
	}
	SetChestSlot(playerid, 0, INV_ITEM_WOOD, 20, "Wood", "Stored building material.", "wood");
	SetChestSlot(playerid, 1, INV_ITEM_STONE, 8, "Stone", "Stored construction material.", "stone");
	SetChestSlot(playerid, 2, INV_ITEM_WATER_BOTTLE, 2, "Water Bottle", "Lootable drink item.", "drink");

	gInventoryInitialised[playerid] = true;
	gInventoryOpen[playerid] = false;
	gWorkspaceInventoryOpen[playerid] = false;
	gStorageOpen[playerid] = false;
	gCraftOpen[playerid] = false;
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
		BuildInventoryDemoActions(playerid, slot, actions, sizeof actions);
		SAMPP_InventorySetSlotActions(playerid, INVENTORY_DEMO_DOC, slot, actions);
	}

	return 1;
}

stock SendWorkspaceInventorySlots(playerid, const documentid[])
{
	for (new slot = 0; slot < INVENTORY_DEMO_SLOTS; slot++)
	{
		if (gInventoryItem[playerid][slot] <= 0 || gInventoryAmount[playerid][slot] <= 0)
		{
			continue;
		}

		SAMPP_WorkspaceSetSlot(
			playerid,
			documentid,
			PANE_INVENTORY,
			slot,
			gInventoryItem[playerid][slot],
			gInventoryAmount[playerid][slot],
			gInventoryName[playerid][slot],
			gInventoryDescription[playerid][slot],
			gInventoryIcon[playerid][slot]
		);

		new actions[128];
		BuildInventoryDemoActions(playerid, slot, actions, sizeof actions);
		SAMPP_WorkspaceSetSlotActions(playerid, documentid, PANE_INVENTORY, slot, actions);
	}
	return 1;
}

stock SendWorkspaceEquipmentSlots(playerid, const documentid[])
{
	for (new slot = 0; slot < EQUIPMENT_DEMO_SLOTS; slot++)
	{
		if (gEquipmentItem[playerid][slot] <= 0 || gEquipmentAmount[playerid][slot] <= 0)
		{
			continue;
		}

		SAMPP_WorkspaceSetSlot(
			playerid,
			documentid,
			PANE_EQUIPMENT,
			slot,
			gEquipmentItem[playerid][slot],
			gEquipmentAmount[playerid][slot],
			gEquipmentName[playerid][slot],
			gEquipmentDescription[playerid][slot],
			gEquipmentIcon[playerid][slot]
		);
		SAMPP_WorkspaceSetSlotActions(playerid, documentid, PANE_EQUIPMENT, slot, "unequip:Unequip;inspect:Inspect");
	}
	return 1;
}

stock SendWorkspaceChestSlots(playerid)
{
	for (new slot = 0; slot < CHEST_DEMO_SLOTS; slot++)
	{
		if (gChestItem[playerid][slot] <= 0 || gChestAmount[playerid][slot] <= 0)
		{
			continue;
		}

		SAMPP_WorkspaceSetSlot(
			playerid,
			STORAGE_WORKSPACE_DOC,
			PANE_CHEST,
			slot,
			gChestItem[playerid][slot],
			gChestAmount[playerid][slot],
			gChestName[playerid][slot],
			gChestDescription[playerid][slot],
			gChestIcon[playerid][slot]
		);
		new actions[128];
		BuildChestDemoActions(playerid, slot, actions, sizeof actions);
		SAMPP_WorkspaceSetSlotActions(playerid, STORAGE_WORKSPACE_DOC, PANE_CHEST, slot, actions);
	}
	return 1;
}

stock RefreshCharacterWorkspace(playerid)
{
	if (!gWorkspaceInventoryOpen[playerid] || !IsUsingSAMPP(playerid))
	{
		return 1;
	}

	SAMPP_WorkspaceClear(playerid, INVENTORY_WORKSPACE_DOC);
	SAMPP_WorkspaceSetPane(playerid, INVENTORY_WORKSPACE_DOC, PANE_EQUIPMENT, SAMPP_PANE_EQUIPMENT, "Character Equipment", EQUIPMENT_DEMO_SLOTS, "Drag compatible items from inventory into equipment slots.");
	SAMPP_WorkspaceSetPane(playerid, INVENTORY_WORKSPACE_DOC, PANE_INVENTORY, SAMPP_PANE_GRID, "Player Inventory", INVENTORY_DEMO_SLOTS, "Server-owned inventory. Drag to equipment, split stacks, or drop supported items.");
	SendWorkspaceEquipmentSlots(playerid, INVENTORY_WORKSPACE_DOC);
	SendWorkspaceInventorySlots(playerid, INVENTORY_WORKSPACE_DOC);
	return 1;
}

stock RefreshStorageWorkspace(playerid)
{
	if (!gStorageOpen[playerid] || !IsUsingSAMPP(playerid))
	{
		return 1;
	}

	SAMPP_WorkspaceClear(playerid, STORAGE_WORKSPACE_DOC);
	SAMPP_WorkspaceSetPane(playerid, STORAGE_WORKSPACE_DOC, PANE_CHEST, SAMPP_PANE_STORAGE, "Loot Chest", CHEST_DEMO_SLOTS, "Drag items between this chest and your inventory.");
	SAMPP_WorkspaceSetPane(playerid, STORAGE_WORKSPACE_DOC, PANE_INVENTORY, SAMPP_PANE_GRID, "Player Inventory", INVENTORY_DEMO_SLOTS, "Server validates every transfer and merge.");
	SendWorkspaceChestSlots(playerid);
	SendWorkspaceInventorySlots(playerid, STORAGE_WORKSPACE_DOC);
	return 1;
}

stock RefreshCraftWorkspace(playerid)
{
	if (!gCraftOpen[playerid] || !IsUsingSAMPP(playerid))
	{
		return 1;
	}

	SAMPP_WorkspaceClear(playerid, CRAFT_WORKSPACE_DOC);
	SAMPP_WorkspaceSetPane(playerid, CRAFT_WORKSPACE_DOC, PANE_RECIPES, SAMPP_PANE_RECIPE_LIST, "Crafting", RECIPE_DEMO_SLOTS, "Select a recipe. Pawn checks materials and output space.");
	SAMPP_WorkspaceSetPane(playerid, CRAFT_WORKSPACE_DOC, PANE_QUEUE, SAMPP_PANE_CRAFT_QUEUE, "Queue / Details", QUEUE_DEMO_SLOTS, "Demo crafts instantly, but real servers can use a queue timer.");
	SAMPP_WorkspaceSetPane(playerid, CRAFT_WORKSPACE_DOC, PANE_INVENTORY, SAMPP_PANE_GRID, "Player Inventory", INVENTORY_DEMO_SLOTS, "Materials are consumed server-side.");

	SAMPP_WorkspaceSetSlot(playerid, CRAFT_WORKSPACE_DOC, PANE_RECIPES, 0, INV_ITEM_FOUNDATION_KIT, 1, "Foundation Kit", "Cost: 1 Wood. Produces a build demo foundation kit.", "craft");
	SAMPP_WorkspaceSetSlotActions(playerid, CRAFT_WORKSPACE_DOC, PANE_RECIPES, 0, "craft:Craft");
	SAMPP_WorkspaceSetSlot(playerid, CRAFT_WORKSPACE_DOC, PANE_RECIPES, 1, INV_ITEM_HATCHET, 1, "Hatchet", "Cost: 2 Wood + 1 Stone. Demonstrates multi-material recipes.", "craft");
	SAMPP_WorkspaceSetSlotActions(playerid, CRAFT_WORKSPACE_DOC, PANE_RECIPES, 1, "craft:Craft");
	SAMPP_WorkspaceSetSlot(playerid, CRAFT_WORKSPACE_DOC, PANE_QUEUE, 0, 9001, 1, "Ready", "Choose a recipe on the left. Results are inserted into inventory if space exists.", "info");

	SendWorkspaceInventorySlots(playerid, CRAFT_WORKSPACE_DOC);
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

stock FindFirstEmptyChestSlot(playerid)
{
	for (new slot = 0; slot < CHEST_DEMO_SLOTS; slot++)
	{
		if (gChestItem[playerid][slot] == 0)
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
	RefreshCharacterWorkspace(playerid);
	RefreshStorageWorkspace(playerid);
	RefreshCraftWorkspace(playerid);
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
	RefreshCharacterWorkspace(playerid);
	RefreshStorageWorkspace(playerid);
	RefreshCraftWorkspace(playerid);
	return true;
}

stock bool:SplitChestStack(playerid, slot, amount)
{
	if (slot < 0 || slot >= CHEST_DEMO_SLOTS || amount <= 0 || gChestItem[playerid][slot] == 0)
	{
		return false;
	}

	if (gChestAmount[playerid][slot] <= 1 || amount >= gChestAmount[playerid][slot])
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[StorageDemo] Split rejected: invalid chest stack amount.");
		return false;
	}

	new target = FindFirstEmptyChestSlot(playerid);
	if (target == -1)
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[StorageDemo] Split rejected: no empty chest slot.");
		return false;
	}

	SetChestSlot(
		playerid,
		target,
		gChestItem[playerid][slot],
		amount,
		gChestName[playerid][slot],
		gChestDescription[playerid][slot],
		gChestIcon[playerid][slot]
	);
	gChestAmount[playerid][slot] -= amount;

	new message[128];
	format(message, sizeof message, "[StorageDemo] split %d item(s) from chest slot %d into slot %d.", amount, slot, target);
	SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, message);
	RefreshStorageWorkspace(playerid);
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
	RefreshCharacterWorkspace(playerid);
	RefreshStorageWorkspace(playerid);
	RefreshCraftWorkspace(playerid);
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

stock InspectEquipmentSlot(playerid, slot)
{
	if (slot < 0 || slot >= EQUIPMENT_DEMO_SLOTS || gEquipmentItem[playerid][slot] == 0)
	{
		return 0;
	}

	new message[160];
	format(
		message,
		sizeof message,
		"[InventoryDemo] equipped %s x%d | itemid=%d | %s",
		gEquipmentName[playerid][slot],
		gEquipmentAmount[playerid][slot],
		gEquipmentItem[playerid][slot],
		gEquipmentDescription[playerid][slot]
	);
	SendClientMessage(playerid, INVENTORY_DEMO_COLOUR, message);
	return 1;
}

stock InspectChestSlot(playerid, slot)
{
	if (slot < 0 || slot >= CHEST_DEMO_SLOTS || gChestItem[playerid][slot] == 0)
	{
		return 0;
	}

	new message[160];
	format(
		message,
		sizeof message,
		"[StorageDemo] chest %s x%d | itemid=%d | %s",
		gChestName[playerid][slot],
		gChestAmount[playerid][slot],
		gChestItem[playerid][slot],
		gChestDescription[playerid][slot]
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
	RefreshCharacterWorkspace(playerid);
	RefreshStorageWorkspace(playerid);
	RefreshCraftWorkspace(playerid);
	SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[InventoryDemo] Water Bottle dropped back into the world.");
	return true;
}

stock bool:IsEquipmentCompatible(itemid, equip_slot)
{
	switch (equip_slot)
	{
		case SAMPP_EQUIP_ARMOR:
		{
			return itemid == INV_ITEM_ARMOR;
		}
		case SAMPP_EQUIP_BACKPACK:
		{
			return itemid == INV_ITEM_BACKPACK;
		}
		case SAMPP_EQUIP_PRIMARY_WEAPON:
		{
			return itemid == INV_ITEM_HAMMER;
		}
		case SAMPP_EQUIP_SECONDARY_WEAPON:
		{
			return itemid == INV_ITEM_AMMO;
		}
	}
	return false;
}

stock bool:MoveInventoryToEquipment(playerid, from_slot, equip_slot)
{
	if (from_slot < 0 || from_slot >= INVENTORY_DEMO_SLOTS || equip_slot < 0 || equip_slot >= EQUIPMENT_DEMO_SLOTS)
	{
		return false;
	}
	if (gInventoryItem[playerid][from_slot] == 0 || !IsEquipmentCompatible(gInventoryItem[playerid][from_slot], equip_slot))
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] This item does not fit that equipment slot.");
		return false;
	}

	new oldItem = gEquipmentItem[playerid][equip_slot];
	new oldAmount = gEquipmentAmount[playerid][equip_slot];
	new oldName[INVENTORY_DEMO_NAME_LEN];
	new oldDescription[INVENTORY_DEMO_DESC_LEN];
	new oldIcon[INVENTORY_DEMO_ICON_LEN];
	format(oldName, sizeof oldName, "%s", gEquipmentName[playerid][equip_slot]);
	format(oldDescription, sizeof oldDescription, "%s", gEquipmentDescription[playerid][equip_slot]);
	format(oldIcon, sizeof oldIcon, "%s", gEquipmentIcon[playerid][equip_slot]);

	SetEquipmentSlot(
		playerid,
		equip_slot,
		gInventoryItem[playerid][from_slot],
		gInventoryAmount[playerid][from_slot],
		gInventoryName[playerid][from_slot],
		gInventoryDescription[playerid][from_slot],
		gInventoryIcon[playerid][from_slot]
	);

	if (oldItem > 0)
	{
		SetInventorySlot(playerid, from_slot, oldItem, oldAmount, oldName, oldDescription, oldIcon);
	}
	else
	{
		ClearInventorySlot(playerid, from_slot);
	}
	return true;
}

stock bool:MoveEquipmentToInventory(playerid, equip_slot, to_slot)
{
	if (equip_slot < 0 || equip_slot >= EQUIPMENT_DEMO_SLOTS || to_slot < 0 || to_slot >= INVENTORY_DEMO_SLOTS || gEquipmentItem[playerid][equip_slot] == 0)
	{
		return false;
	}
	if (gInventoryItem[playerid][to_slot] != 0)
	{
		SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] Unequip target slot must be empty in this demo.");
		return false;
	}

	SetInventorySlot(
		playerid,
		to_slot,
		gEquipmentItem[playerid][equip_slot],
		gEquipmentAmount[playerid][equip_slot],
		gEquipmentName[playerid][equip_slot],
		gEquipmentDescription[playerid][equip_slot],
		gEquipmentIcon[playerid][equip_slot]
	);
	ClearEquipmentSlot(playerid, equip_slot);
	return true;
}

stock bool:MoveInventoryToChest(playerid, from_slot, to_slot)
{
	if (from_slot < 0 || from_slot >= INVENTORY_DEMO_SLOTS || to_slot < 0 || to_slot >= CHEST_DEMO_SLOTS || gInventoryItem[playerid][from_slot] == 0)
	{
		return false;
	}
	if (gChestItem[playerid][to_slot] == 0)
	{
		SetChestSlot(playerid, to_slot, gInventoryItem[playerid][from_slot], gInventoryAmount[playerid][from_slot], gInventoryName[playerid][from_slot], gInventoryDescription[playerid][from_slot], gInventoryIcon[playerid][from_slot]);
		ClearInventorySlot(playerid, from_slot);
		return true;
	}
	if (gChestItem[playerid][to_slot] == gInventoryItem[playerid][from_slot])
	{
		gChestAmount[playerid][to_slot] += gInventoryAmount[playerid][from_slot];
		ClearInventorySlot(playerid, from_slot);
		return true;
	}
	SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[StorageDemo] Target chest slot is occupied by another item.");
	return false;
}

stock bool:MoveChestToInventory(playerid, from_slot, to_slot)
{
	if (from_slot < 0 || from_slot >= CHEST_DEMO_SLOTS || to_slot < 0 || to_slot >= INVENTORY_DEMO_SLOTS || gChestItem[playerid][from_slot] == 0)
	{
		return false;
	}
	if (gInventoryItem[playerid][to_slot] == 0)
	{
		SetInventorySlot(playerid, to_slot, gChestItem[playerid][from_slot], gChestAmount[playerid][from_slot], gChestName[playerid][from_slot], gChestDescription[playerid][from_slot], gChestIcon[playerid][from_slot]);
		ClearChestSlot(playerid, from_slot);
		return true;
	}
	if (gInventoryItem[playerid][to_slot] == gChestItem[playerid][from_slot])
	{
		gInventoryAmount[playerid][to_slot] += gChestAmount[playerid][from_slot];
		ClearChestSlot(playerid, from_slot);
		return true;
	}
	SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[StorageDemo] Target inventory slot is occupied by another item.");
	return false;
}

stock CountInventoryItem(playerid, itemid)
{
	new total = 0;
	for (new slot = 0; slot < INVENTORY_DEMO_SLOTS; slot++)
	{
		if (gInventoryItem[playerid][slot] == itemid)
		{
			total += gInventoryAmount[playerid][slot];
		}
	}
	return total;
}

stock bool:ConsumeInventoryItem(playerid, itemid, amount)
{
	if (amount <= 0 || CountInventoryItem(playerid, itemid) < amount)
	{
		return false;
	}
	for (new slot = 0; slot < INVENTORY_DEMO_SLOTS && amount > 0; slot++)
	{
		if (gInventoryItem[playerid][slot] != itemid)
		{
			continue;
		}
		new take = gInventoryAmount[playerid][slot] < amount ? gInventoryAmount[playerid][slot] : amount;
		gInventoryAmount[playerid][slot] -= take;
		amount -= take;
		if (gInventoryAmount[playerid][slot] <= 0)
		{
			ClearInventorySlot(playerid, slot);
		}
	}
	return true;
}

stock bool:CraftRecipe(playerid, recipe_slot)
{
	if (recipe_slot == 0)
	{
		if (!ConsumeInventoryItem(playerid, INV_ITEM_WOOD, 1))
		{
			SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[CraftDemo] Need 1 Wood for Foundation Kit.");
			return false;
		}
		AddInventoryDemoItem(playerid, INV_ITEM_FOUNDATION_KIT, 1, "Foundation Kit", "Crafted demo output. A real mode would feed the build system.", "craft");
		SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[CraftDemo] Crafted Foundation Kit.");
		return true;
	}
	if (recipe_slot == 1)
	{
		if (CountInventoryItem(playerid, INV_ITEM_WOOD) < 2 || CountInventoryItem(playerid, INV_ITEM_STONE) < 1)
		{
			SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[CraftDemo] Need 2 Wood and 1 Stone for Hatchet.");
			return false;
		}
		ConsumeInventoryItem(playerid, INV_ITEM_WOOD, 2);
		ConsumeInventoryItem(playerid, INV_ITEM_STONE, 1);
		AddInventoryDemoItem(playerid, INV_ITEM_HATCHET, 1, "Hatchet", "Crafted tool demo output.", "tool");
		SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[CraftDemo] Crafted Hatchet.");
		return true;
	}
	return false;
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

stock OpenCharacterInventoryDemo(playerid)
{
	if (!RequireInventoryDemo(playerid))
	{
		return 1;
	}

	if (!gInventoryInitialised[playerid])
	{
		ResetInventoryDemo(playerid);
	}

	SAMPP_WorkspaceOpen(
		playerid,
		INVENTORY_WORKSPACE_DOC,
		SAMPP_WORKSPACE_LAYOUT_INVENTORY,
		"Character Inventory",
		"Equipment is a fixed pane. Inventory is a normal grid. Pawn validates every move."
	);

	gWorkspaceInventoryOpen[playerid] = true;
	RefreshCharacterWorkspace(playerid);
	SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[InventoryDemo] Character inventory opened. Drag armor/backpack/hammer into compatible equipment slots.");
	return 1;
}

stock OpenStorageDemo(playerid)
{
	if (!RequireInventoryDemo(playerid))
	{
		return 1;
	}

	if (!gInventoryInitialised[playerid])
	{
		ResetInventoryDemo(playerid);
	}

	SAMPP_WorkspaceOpen(
		playerid,
		STORAGE_WORKSPACE_DOC,
		SAMPP_WORKSPACE_LAYOUT_STORAGE,
		"Storage Workspace",
		"Left pane is a server-owned chest. Right pane is the player inventory."
	);

	gStorageOpen[playerid] = true;
	RefreshStorageWorkspace(playerid);
	SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[StorageDemo] Chest opened. Drag items between chest and inventory.");
	return 1;
}

stock OpenCraftDemo(playerid)
{
	if (!RequireInventoryDemo(playerid))
	{
		return 1;
	}

	if (!gInventoryInitialised[playerid])
	{
		ResetInventoryDemo(playerid);
	}

	SAMPP_WorkspaceOpen(
		playerid,
		CRAFT_WORKSPACE_DOC,
		SAMPP_WORKSPACE_LAYOUT_CRAFTING,
		"Crafting Workspace",
		"Recipes, queue/details, and inventory use one workspace document."
	);

	gCraftOpen[playerid] = true;
	RefreshCraftWorkspace(playerid);
	SendClientMessage(playerid, INVENTORY_DEMO_OK_COLOUR, "[CraftDemo] Crafting opened. Click a recipe to craft if materials are available.");
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
		return OpenCharacterInventoryDemo(playerid);
	}

	if (!strcmp(cmdtext, "/inventorysimple", true) || !strcmp(cmdtext, "/invdemo_simple", true))
	{
		return OpenInventoryDemo(playerid);
	}

	if (!strcmp(cmdtext, "/storagedemo", true) || !strcmp(cmdtext, "/chestdemo", true))
	{
		return OpenStorageDemo(playerid);
	}

	if (!strcmp(cmdtext, "/craftdemo", true))
	{
		return OpenCraftDemo(playerid);
	}

	if (!strcmp(cmdtext, "/inventoryclose", true) || !strcmp(cmdtext, "/invclose", true))
	{
		gInventoryOpen[playerid] = false;
		gWorkspaceInventoryOpen[playerid] = false;
		gStorageOpen[playerid] = false;
		gCraftOpen[playerid] = false;
		SAMPP_UIClose(playerid, INVENTORY_DEMO_DOC);
		SAMPP_UIClose(playerid, INVENTORY_WORKSPACE_DOC);
		SAMPP_UIClose(playerid, STORAGE_WORKSPACE_DOC);
		SAMPP_UIClose(playerid, CRAFT_WORKSPACE_DOC);
		return 1;
	}

	if (!strcmp(cmdtext, "/inventoryreset", true) || !strcmp(cmdtext, "/invreset", true))
	{
		ResetInventoryDemo(playerid);
		gWorkspaceInventoryOpen[playerid] = true;
		OpenCharacterInventoryDemo(playerid);
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

public OnPlayerOMPPlusWorkspaceDrop(playerid, documentid[], from_pane[], from_slot, to_pane[], to_slot, amount, payload[])
{
	if (!strcmp(documentid, INVENTORY_WORKSPACE_DOC, true))
	{
		if (!strcmp(from_pane, PANE_INVENTORY, true) && !strcmp(to_pane, "world", true))
		{
			DropInventorySlotToWorld(playerid, from_slot);
		}
		else if (!strcmp(from_pane, PANE_INVENTORY, true) && !strcmp(to_pane, PANE_EQUIPMENT, true))
		{
			MoveInventoryToEquipment(playerid, from_slot, to_slot);
		}
		else if (!strcmp(from_pane, PANE_EQUIPMENT, true) && !strcmp(to_pane, PANE_INVENTORY, true))
		{
			MoveEquipmentToInventory(playerid, from_slot, to_slot);
		}
		else if (!strcmp(from_pane, PANE_INVENTORY, true) && !strcmp(to_pane, PANE_INVENTORY, true))
		{
			MoveInventorySlot(playerid, from_slot, to_slot);
		}
		RefreshCharacterWorkspace(playerid);
		return 1;
	}

	if (!strcmp(documentid, STORAGE_WORKSPACE_DOC, true))
	{
		if (!strcmp(from_pane, PANE_INVENTORY, true) && !strcmp(to_pane, "world", true))
		{
			DropInventorySlotToWorld(playerid, from_slot);
		}
		else if (!strcmp(from_pane, PANE_INVENTORY, true) && !strcmp(to_pane, PANE_CHEST, true))
		{
			MoveInventoryToChest(playerid, from_slot, to_slot);
		}
		else if (!strcmp(from_pane, PANE_CHEST, true) && !strcmp(to_pane, PANE_INVENTORY, true))
		{
			MoveChestToInventory(playerid, from_slot, to_slot);
		}
		else if (!strcmp(from_pane, PANE_INVENTORY, true) && !strcmp(to_pane, PANE_INVENTORY, true))
		{
			MoveInventorySlot(playerid, from_slot, to_slot);
		}
		RefreshStorageWorkspace(playerid);
		return 1;
	}

	if (!strcmp(documentid, CRAFT_WORKSPACE_DOC, true))
	{
		if (!strcmp(from_pane, PANE_INVENTORY, true) && !strcmp(to_pane, "world", true))
		{
			DropInventorySlotToWorld(playerid, from_slot);
		}
		else if (!strcmp(from_pane, PANE_INVENTORY, true) && !strcmp(to_pane, PANE_INVENTORY, true))
		{
			MoveInventorySlot(playerid, from_slot, to_slot);
			RefreshCraftWorkspace(playerid);
		}
		return 1;
	}

	return 1;
}

public OnPlayerOMPPlusWorkspaceAction(playerid, documentid[], paneid[], slot, action[], payload[])
{
	if (!strcmp(documentid, CRAFT_WORKSPACE_DOC, true) && !strcmp(paneid, PANE_RECIPES, true))
	{
		if (!strcmp(action, "craft", true) || !strcmp(action, "select", true))
		{
			CraftRecipe(playerid, slot);
			RefreshCraftWorkspace(playerid);
		}
		return 1;
	}

	if (!strcmp(documentid, INVENTORY_WORKSPACE_DOC, true))
	{
		if (!strcmp(paneid, PANE_EQUIPMENT, true) && !strcmp(action, "unequip", true))
		{
			new target = FindFirstEmptyInventorySlot(playerid);
			if (target != -1)
			{
				MoveEquipmentToInventory(playerid, slot, target);
			}
			RefreshCharacterWorkspace(playerid);
		}
		else if (!strcmp(paneid, PANE_EQUIPMENT, true) && !strcmp(action, "inspect", true))
		{
			InspectEquipmentSlot(playerid, slot);
		}
		else if (!strcmp(paneid, PANE_INVENTORY, true))
		{
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
			RefreshCharacterWorkspace(playerid);
		}
		return 1;
	}

	if (!strcmp(documentid, STORAGE_WORKSPACE_DOC, true))
	{
		if (!strcmp(paneid, PANE_CHEST, true) && !strcmp(action, "take", true))
		{
			new target = FindFirstEmptyInventorySlot(playerid);
			if (target != -1)
			{
				MoveChestToInventory(playerid, slot, target);
			}
			RefreshStorageWorkspace(playerid);
		}
		else if (!strcmp(paneid, PANE_CHEST, true) && !strcmp(action, "inspect", true))
		{
			InspectChestSlot(playerid, slot);
		}
		else if (!strcmp(paneid, PANE_INVENTORY, true))
		{
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
			RefreshStorageWorkspace(playerid);
		}
		return 1;
	}

	if (!strcmp(documentid, CRAFT_WORKSPACE_DOC, true))
	{
		if (!strcmp(paneid, PANE_INVENTORY, true))
		{
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
			RefreshCraftWorkspace(playerid);
		}
		return 1;
	}

	return 1;
}

public OnPlayerOMPPlusWorkspaceSplit(playerid, documentid[], paneid[], slot, amount, payload[])
{
	if (!strcmp(paneid, PANE_INVENTORY, true))
	{
		if (!strcmp(documentid, INVENTORY_WORKSPACE_DOC, true) || !strcmp(documentid, STORAGE_WORKSPACE_DOC, true) || !strcmp(documentid, CRAFT_WORKSPACE_DOC, true))
		{
			SplitInventoryStack(playerid, slot, amount);
		}
		return 1;
	}

	if (!strcmp(documentid, STORAGE_WORKSPACE_DOC, true) && !strcmp(paneid, PANE_CHEST, true))
	{
		SplitChestStack(playerid, slot, amount);
		return 1;
	}

	SendClientMessage(playerid, INVENTORY_DEMO_WARN_COLOUR, "[InventoryDemo] Split rejected: this pane does not support stack splitting.");
	return 1;
}

public OnPlayerOMPPlusUIEvent(playerid, documentid[], event_type, slot, element[], payload[])
{
	if (!strcmp(documentid, INVENTORY_DEMO_DOC, true) && event_type == SAMPP_UI_EVENT_CLOSE)
	{
		gInventoryOpen[playerid] = false;
		SendClientMessage(playerid, INVENTORY_DEMO_COLOUR, "[InventoryDemo] Inventory closed from the client.");
	}
	else if (!strcmp(documentid, INVENTORY_WORKSPACE_DOC, true) && event_type == SAMPP_UI_EVENT_CLOSE)
	{
		gWorkspaceInventoryOpen[playerid] = false;
	}
	else if (!strcmp(documentid, STORAGE_WORKSPACE_DOC, true) && event_type == SAMPP_UI_EVENT_CLOSE)
	{
		gStorageOpen[playerid] = false;
	}
	else if (!strcmp(documentid, CRAFT_WORKSPACE_DOC, true) && event_type == SAMPP_UI_EVENT_CLOSE)
	{
		gCraftOpen[playerid] = false;
	}
	return 1;
}
