# RmlUi / Panel UI Bridge

OpenMP-Plus keeps ImGui for target, build, and developer tools. Larger player-facing interfaces use the new UI bridge API, so Pawn scripts can open inventory, storage, crafting, phone, tablet, and full-screen panels without touching the ImGui target/build code.

The current client project still targets the legacy VS2013 `v120` toolset, while upstream RmlUi requires C++17. For that reason the first integration ships with a non-ImGui D3D9 fallback renderer behind the `CRmlUiManager` boundary and the RmlUi submodule available under `ThirdParty/RmlUi` for a later modern-toolset backend. The Pawn API and wire protocol are already RmlUi-shaped and will not need to change when the real backend is enabled.

## Capability

Check the client first:

```pawn
if (!SAMPP_HasRmlUi(playerid))
{
    SendClientMessage(playerid, -1, "This UI needs the OpenMP-Plus RmlUi capability.");
    return 1;
}
```

The capability bit is:

```pawn
SAMPP_CAPABILITY_RMLUI
```

It is separate from `SAMPP_FEATURE_UI`, because `SAMPP_FEATURE_UI` is a general feature flag and `SAMPP_CAPABILITY_RMLUI` means the client can open the larger panel UI channel.

## Inventory Example

```pawn
SAMPP_InventoryOpen(
    playerid,
    "inventory",
    "Survival Inventory",
    30,
    "Pawn owns all item state. The client only renders and reports clicks."
);

SAMPP_InventoryClear(playerid, "inventory");
SAMPP_InventorySetSlot(playerid, "inventory", 0, 1001, 4, "Wood", "Basic building material.", "wood");
SAMPP_InventorySetSlotActions(playerid, "inventory", 0, "use:Use;split:Split Stack;drop:Drop;inspect:Inspect");
SAMPP_InventorySetSlot(playerid, "inventory", 1, 2001, 1, "Bandage", "Restores a small amount of health.", "medical");
SAMPP_InventorySetSlotActions(playerid, "inventory", 1, "use:Use;drop:Drop;inspect:Inspect");
```

Clicks return to Pawn:

```pawn
public OnPlayerOMPPlusInventoryClick(playerid, documentid[], slot, event_type, payload[])
{
    if (!strcmp(documentid, "inventory", true))
    {
        // Validate the slot server-side, then run your inventory action.
    }
    return 1;
}
```

Drag/drop returns to Pawn as a separate event:

```pawn
public OnPlayerOMPPlusInventoryDrop(playerid, documentid[], from_slot, to_slot, payload[])
{
    if (strcmp(documentid, "inventory", true))
    {
        return 1;
    }

    if (to_slot == -1)
    {
        // The player dragged the item outside the panel.
        // Validate ownership and spawn/drop the item server-side.
        return 1;
    }

    // Validate from_slot and to_slot, then move, merge, or swap server-side.
    return 1;
}
```

The client sends these event types:

```pawn
SAMPP_UI_EVENT_SLOT_DROP   // from_slot -> to_slot
SAMPP_UI_EVENT_WORLD_DROP  // from_slot -> outside the document, to_slot = -1
```

Right-click slot actions are server-configured per slot:

```pawn
SAMPP_InventorySetSlotActions(
    playerid,
    "inventory",
    0,
    "use:Use;split:Split Stack;drop:Drop;inspect:Inspect"
);
```

The format is `action_id:Visible Label`, separated by semicolons. The action id
is what Pawn receives; the visible label is only UI text. Keep ids stable and
validate every request against your real item state.

```pawn
public OnPlayerOMPPlusInventoryAction(playerid, documentid[], slot, action[], payload[])
{
    if (strcmp(documentid, "inventory", true))
    {
        return 1;
    }

    if (!strcmp(action, "use", true))
    {
        // Validate item ownership, cooldowns, and item type, then consume/use.
    }
    else if (!strcmp(action, "drop", true))
    {
        // Spawn the item server-side and remove it from the slot.
    }
    return 1;
}
```

The built-in `split` action opens a client-side amount dialog and then reports
the selected amount back to Pawn:

```pawn
public OnPlayerOMPPlusInventorySplit(playerid, documentid[], slot, amount, payload[])
{
    if (!strcmp(documentid, "inventory", true))
    {
        // Validate that the slot is stackable and amount is less than the stack.
    }
    return 1;
}
```

`payload` is intentionally informational. The first field is the target slot
for slot drops, followed by item id, amount, and label for debugging. Do not
trust it for item ownership. Read the real item from your Pawn inventory state.

The bundled `sampp_inventorydemo.pwn` shows complete server-owned examples:

- `/inventorydemo` opens the character workspace: equipment on the left,
  inventory on the right.
- `/inventorysimple` opens the legacy single-grid inventory panel.
- `/storagedemo` opens a storage/chest workspace: chest on the left, player
  inventory on the right.
- `/craftdemo` opens a crafting workspace: recipe list, queue/details, and
  player inventory.
- Dragging between slots moves, merges matching item IDs, or swaps different
  items.
- Dragging a Water Bottle outside the panel calls the item demo and recreates
  the world item.
- Right-clicking a slot opens server-defined actions such as `Use`, `Split
  Stack`, `Drop`, and `Inspect`.
- `Split Stack` opens an amount selector; Pawn creates the new stack only after
  server-side validation.
- `/itemadd` from `sampp_itemdemo.pwn` creates a Water Bottle world object; when
  picked up with `E`, it is inserted into the inventory demo.

## Workspace API

Use workspaces for multi-panel UIs such as equipment, storage, loot, trade, or
crafting. A workspace is one document with multiple named panes. Each pane owns
its own slots, type, title, and actions.

```pawn
SAMPP_WorkspaceOpen(
    playerid,
    "character",
    SAMPP_WORKSPACE_LAYOUT_INVENTORY,
    "Character Inventory",
    "Equipment and bag inventory in one server-owned UI."
);

SAMPP_WorkspaceClear(playerid, "character");
SAMPP_WorkspaceSetPane(playerid, "character", "equipment", SAMPP_PANE_EQUIPMENT, "Equipment", 10);
SAMPP_WorkspaceSetPane(playerid, "character", "inventory", SAMPP_PANE_GRID, "Inventory", 30);
SAMPP_WorkspaceSetSlot(playerid, "character", "inventory", 0, 6001, 1, "Kevlar Vest", "Drag to Armor.", "armor");
SAMPP_WorkspaceSetSlotActions(playerid, "character", "inventory", 0, "use:Use;inspect:Inspect");
```

Pane types:

```pawn
SAMPP_PANE_GRID
SAMPP_PANE_EQUIPMENT
SAMPP_PANE_STORAGE
SAMPP_PANE_LOOT
SAMPP_PANE_RECIPE_LIST
SAMPP_PANE_CRAFT_QUEUE
SAMPP_PANE_INFO
```

Layout hints:

```pawn
SAMPP_WORKSPACE_LAYOUT_INVENTORY
SAMPP_WORKSPACE_LAYOUT_STORAGE
SAMPP_WORKSPACE_LAYOUT_CRAFTING
SAMPP_WORKSPACE_LAYOUT_TRADE
```

Workspace drag/drop includes pane ids, so one callback can handle inventory,
equipment, chest, loot, and trade transfers:

```pawn
public OnPlayerOMPPlusWorkspaceDrop(
    playerid,
    documentid[],
    from_pane[],
    from_slot,
    to_pane[],
    to_slot,
    amount,
    payload[]
)
{
    // Validate from_pane/from_slot -> to_pane/to_slot server-side.
    // Example: inventory -> equipment requires item category compatibility.
    return 1;
}
```

Dragging from a workspace pane outside the document reports `to_pane` as
`"world"` and `to_slot` as `-1`. Treat that the same way you would a normal
world drop: validate the source pane and slot server-side, then spawn or reject
the world item.

Workspace actions are also pane-aware:

```pawn
public OnPlayerOMPPlusWorkspaceAction(playerid, documentid[], paneid[], slot, action[], payload[])
{
    if (!strcmp(paneid, "recipes", true) && !strcmp(action, "craft", true))
    {
        // Check materials, queue limits, workbench access, and output space.
    }
    return 1;
}
```

Workspace stack splitting is pane-aware too. If a slot action contains
`split:Split Stack`, the client opens the amount selector and reports the
selected amount with the pane id:

```pawn
public OnPlayerOMPPlusWorkspaceSplit(playerid, documentid[], paneid[], slot, amount, payload[])
{
    if (!strcmp(paneid, "inventory", true))
    {
        // Validate the stack and amount, then create the new stack server-side.
    }
    return 1;
}
```

The client never moves items by itself. It sends a request; Pawn changes the
real arrays/database and refreshes affected panes or slots.

### Modular Workspace Patterns

Workspaces are intentionally pane-based. A server can send only the panes it
needs for the current feature:

- Inventory only: one `SAMPP_PANE_GRID` pane.
- Character inventory: `SAMPP_PANE_EQUIPMENT` plus `SAMPP_PANE_GRID`.
- Storage/chest: `SAMPP_PANE_STORAGE` plus `SAMPP_PANE_GRID`.
- Looting another player: `SAMPP_PANE_LOOT` plus `SAMPP_PANE_GRID`.
- Crafting: `SAMPP_PANE_RECIPE_LIST`, optional `SAMPP_PANE_CRAFT_QUEUE`, and
  optional inventory/material panes.
- Trade: two grid/storage panes and server-side accept/lock state through
  normal `SAMPP_UISetData` or custom pane actions.

Do not build separate client code for every UI. Keep the client as a generic
workspace renderer and let Pawn decide which panes, slots, actions, and layout
hints are active for a specific server system.

## Generic Panel

```pawn
SAMPP_UIOpen(
    playerid,
    "base_panel",
    SAMPP_UI_TEMPLATE_PANEL,
    "Base Management",
    "Manage upkeep, members, and decay state."
);

SAMPP_UISetData(playerid, "base_panel", "Owner", "cengizhanbucak");
SAMPP_UISetData(playerid, "base_panel", "Upkeep", "24 hours");
```

## Security Rule

The client must be treated as a renderer only. Inventory contents, storage
contents, crafting results, permissions, currency, item movement, and item use
are all server-authoritative. Every UI click and drag/drop is only a request.
