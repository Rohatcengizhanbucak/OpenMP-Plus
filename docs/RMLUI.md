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
SAMPP_InventorySetSlot(playerid, "inventory", 1, 2001, 1, "Bandage", "Restores a small amount of health.", "medical");
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

The client must be treated as a renderer only. Inventory contents, storage contents, crafting results, permissions, currency, item movement, and item use are all server-authoritative. Every UI click is only a request.
