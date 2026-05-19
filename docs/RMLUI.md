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

The bundled `sampp_inventorydemo.pwn` shows a complete server-owned example:

- `/inventorydemo` opens the panel.
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
