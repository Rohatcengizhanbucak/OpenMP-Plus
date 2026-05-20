# OpenMP-Plus

OpenMP-Plus is an open.mp-focused port and modernization branch of the archived SA-MP+ project.

The current architecture has a native open.mp component transport that uses open.mp's existing `INetwork` / RakServer pipeline. The old SA-MP+ `server_port + 1` side-channel is still present as a legacy fallback, but it is no longer the target transport.

## Current Status

- Native open.mp component build target: `Build/Release/omp-plus.dll`
- Windows open.mp legacy plugin fallback: `Build/Release/sampp_server.dll` only when explicitly built with `OMPPLUS_BUILD_LEGACY_PLUGIN=ON`
- Safe client ASI: `Build/Release/sampp_client.asi`
- PAWN include: `Build/sampp.inc`
- Native transport: custom RPC on the existing game connection, no extra UDP port
- Legacy side-channel fallback: `server_port + 1`, for example `7778` when the game server uses `7777`
- Safe client mode avoids Direct3D and DirectInput hooks and only enables the verified HUD/keybind RPC subset by default.
- Client native transport source currently targets known SA-MP/open.mp 0.3.7 and 0.3DL `samp.dll` entry points, with guarded pointer and vtable validation before using the game RakClient interface.
- Native transport reserves custom RPC `220`; do not reuse that RPC ID in other client/server extensions loaded in the same session.

Prebuilt Windows binaries may need to be rebuilt from this source tree before they include the native component transport. Linux binaries are not shipped yet for this port because the old `sampp_server.so` artifact was stale and has been removed until it can be rebuilt and tested.

## Verified Features

- Native open.mp component loading through `components\omp-plus.dll`
- Native RPC `220` handshake over the existing player connection
- Legacy fallback config reader with open.mp `config.json` and legacy `server.cfg`
- `IsUsingSAMPP(playerid)` compatibility native detection
- HUD component toggle RPCs
- Safe keybind callbacks using WinAPI keyboard polling
- Keybind callbacks are suppressed while SA-MP chat input is active.
- Server-driven target UI and experimental build UI demos through the native
  RPC bridge.
- RmlUi-oriented panel bridge for larger Pawn-driven interfaces such as
  inventory, storage, crafting, phone/tablet, and base management.
- `OnPlayerSAMPPKey(playerid, keyid, keystate, action[])`

Previous smoke tests confirmed the safe feature subset:

- `/sampp` reports `IsUsingSAMPP=1`
- `/samppmoney` toggles money HUD
- `/samppammo` toggles ammo HUD
- `/samppweapon` toggles the weapon icon
- `/samppmap` toggles minimap
- `B` triggers `action=money`
- `F2` triggers `action=help`

## Installation

Install the server component and the client ASI from the same build or release. A
new client ASI talking to an old server DLL, or the opposite, can leave
`IsUsingSAMPP(playerid)` at `0` because the native RPC protocol does not match.

Builds that include the native component require the open.mp SDK submodules:

```bat
git submodule update --init --recursive
```

Detailed setup notes are also available in [docs/INSTALL.md](docs/INSTALL.md).
The experimental build UI demo is documented in [docs/BUILD_DEMO.md](docs/BUILD_DEMO.md).
The large panel UI bridge is documented in [docs/RMLUI.md](docs/RMLUI.md).

### Server

Native open.mp mode installs these files on the server:

```text
<openmp-server>\components\omp-plus.dll
<openmp-server>\qawno\include\sampp.inc
```

Optional smoke-test files:

```text
<openmp-server>\filterscripts\sampp_smoketest.amx
<openmp-server>\filterscripts\sampp_itemdemo.amx
<openmp-server>\filterscripts\sampp_menudemo.amx
<openmp-server>\filterscripts\sampp_capabilitydemo.amx
<openmp-server>\filterscripts\sampp_targetdemo.amx
<openmp-server>\filterscripts\sampp_builddemo.amx
```

Do not add a top-level `components` list containing only `omp-plus`. On some
open.mp server packages that disables the default component set, including the
Pawn component. The recommended install is to copy `omp-plus.dll` into the
`components` directory and leave the top-level `components` key absent.

If your server intentionally uses an explicit top-level `components` list, add
`omp-plus` to that full list alongside every default component your package
needs. Do not replace the list with only `omp-plus`.

Legacy side-channel fallback only: build with `OMPPLUS_BUILD_LEGACY_PLUGIN=ON`,
then copy `Build/Release/sampp_server.dll` to
`plugins` and add it as a legacy plugin:

```json
"pawn": {
    "legacy_plugins": [
        "sampp_server"
    ]
}
```

Add your gamemode or filterscript as usual. To use the smoke-test filterscript,
copy the `.amx` file to `filterscripts` and add it to `pawn.side_scripts`.

The legacy fallback plugin reads these open.mp keys:

- `network.port`
- `network.bind`
- `max_players`

It also keeps legacy fallback support for:

- `port`
- `bind`
- `maxplayers`

### Client

Native client mode installs these files in the GTA San Andreas folder that
actually launches the game:

```text
<gta-sa>\sampp_client.asi
<gta-sa>\<ASI loader files, if the game does not already load ASI plugins>
```

The server does not send the ASI to players. Each player who should use
OpenMP-Plus features must have `sampp_client.asi` in the same directory as the
`gta_sa.exe` they launch. If you use a launcher or client manager, verify which
GTA folder it starts before copying the ASI.

Install an ASI loader for GTA San Andreas if one is not already installed. The
loader package can use different proxy DLL names depending on the loader build.
Common layouts include `vorbisFile.dll` plus `vorbisHooked.dll`, or a proxy such
as `dinput8.dll` or `version.dll`. The important rule is that ASI files in the
GTA folder must actually load at game startup.

After copying the client files:

1. Join the open.mp server.
2. Use `/sampp` in-game.
3. Confirm that the server reports `IsUsingSAMPP=1`.

If `/sampp` reports `IsUsingSAMPP=0`, check that:

- `sampp_client.asi` is in the launched GTA folder, not only in a different
  game copy.
- An ASI loader is installed and loading ASI plugins.
- The server has the matching new `components\omp-plus.dll`.
- The server config still loads the Pawn component.

The ASI now defaults to native RakClient transport. To force the old side-channel while testing, pass `-sampp_legacy_sidechannel`; this re-enables the `server_port + 1` client connection.

No installer is currently shipped. The old SA-MP+ installer was removed because it targeted the archived project and could install unsafe or outdated client files.

## Smoke Test

Example smoke-test files are provided in:

```text
examples/filterscripts/sampp_smoketest.pwn
examples/filterscripts/sampp_smoketest.amx
examples/filterscripts/sampp_itemdemo.pwn
examples/filterscripts/sampp_itemdemo.amx
examples/filterscripts/sampp_menudemo.pwn
examples/filterscripts/sampp_menudemo.amx
examples/filterscripts/sampp_capabilitydemo.pwn
examples/filterscripts/sampp_capabilitydemo.amx
examples/filterscripts/sampp_targetdemo.pwn
examples/filterscripts/sampp_targetdemo.amx
examples/filterscripts/sampp_builddemo.pwn
examples/filterscripts/sampp_builddemo.amx
examples/filterscripts/sampp_inventorydemo.pwn
examples/filterscripts/sampp_inventorydemo.amx
```

Copy the `.amx` files you want to test to your open.mp server `filterscripts` directory, then add them to your open.mp config:

```json
"pawn": {
    "side_scripts": [
        "filterscripts/sampp_smoketest",
        "filterscripts/sampp_itemdemo",
        "filterscripts/sampp_menudemo",
        "filterscripts/sampp_capabilitydemo",
        "filterscripts/sampp_targetdemo",
        "filterscripts/sampp_builddemo",
        "filterscripts/sampp_inventorydemo"
    ]
}
```

Smoke-test commands:

- `/sampp`
- `/sampphelp`
- `/samppkeys`
- `/samppmoney` or `/sampphud`
- `/samppammo`
- `/samppweapon`
- `/sampphealth`
- `/samppbreath`
- `/sampparmour` or `/sampparmor`
- `/samppmap`
- `/samppcrosshair`
- `/samppall`

Smoke-test keybinds:

- `F2`: show help
- `B`: toggle money HUD

Item demo commands:

- `/itemadd`: create a Water Bottle object at your position
- `/itemkeys`: reset the contextual `E` capture state
- `/itempickup`: fallback command for pickup testing
- `/itemclear`: remove all demo items

Item demo keybind:

- `E`: pick up the nearest Water Bottle through a short-lived capture lease.
  GTA's default `E` behavior is consumed only while the player is near the item.
  When `sampp_inventorydemo` is loaded, the picked-up Water Bottle is added to
  the server-owned inventory demo instead of being only deleted from the world.

Inventory demo commands:

- `/inventorydemo` or `/invdemo`: open the character workspace. Equipment is
  shown on the left, player inventory on the right.
- `/inventorysimple` or `/invdemo_simple`: open the older single-grid
  inventory panel for compatibility testing.
- `/storagedemo` or `/chestdemo`: open a storage workspace. The chest/storage
  pane and the player inventory pane can exchange items through drag/drop.
- `/craftdemo`: open a crafting workspace with recipes, queue/details, and
  player inventory panes.
- `/inventoryclose` or `/invclose`: close the inventory panel from Pawn.
- `/inventoryreset` or `/invreset`: reset the demo inventory to its default
  items.

Inventory demo flow:

- Drag a used slot onto another slot to move, merge matching item IDs, or swap
  different items. The client only reports the drag request; Pawn owns the item
  arrays and refreshes the UI.
- Right-click a used slot to open its Pawn-defined action menu. The bundled
  demo exposes `Use`, `Split Stack`, `Drop`, and `Inspect` where the item type
  supports them.
- `Split Stack` opens a small amount selector in both the single inventory and
  workspace panes. The client only sends the requested amount plus pane context;
  Pawn validates the source stack and creates the new stack.
- Drag a Water Bottle outside the single inventory panel or a workspace
  inventory pane to drop it back into the world through `sampp_itemdemo`. Walk
  near it and press `E` to pick it up into the inventory again.
- In `/inventorydemo`, drag compatible items such as Kevlar Vest or Backpack
  into equipment slots. Pawn validates the target slot type before changing the
  real item state.
- In `/storagedemo`, drag items between the storage pane and inventory pane.
  This is the same flow a server can reuse for loot chests, trunks, lockers, or
  another player's loot inventory.
- In `/craftdemo`, select a recipe such as Foundation Kit or Hatchet. Pawn
  checks materials, consumes them, and inserts the output into inventory. The
  client only displays the workspace and reports the requested recipe action.

Menu demo commands:

- `/menutoggle` or `/menu`: open/close the TextDraw menu without the keybind
- `/menukeys`: rebind `M` through SA-MP+
- `/menuhelp`: list menu-demo commands

Menu demo keybind:

- `M`: open/close a player TextDraw menu through `OnPlayerSAMPPKey`

Capability/context demo commands:

- `/capinfo` or `/ompinfo`: show negotiated client version, feature flags,
  capabilities, hash prefix, and launcher verification flag
- `/capspawn`: create one demo item and one demo vehicle for the player
- `/capitem`: create only the contextual pickup item
- `/capveh`: create only the contextual vehicle
- `/cappickup`, `/capenter`, `/capengine`, `/capbonnet`, `/capboot`,
  `/capdoors`, `/caplock`: fallback commands when key capture is unavailable
- `/capclear`: remove the player's demo item, vehicle, and active capture leases

Capability/context demo keybind:

- `E`: context-sensitive interaction. Near the item it picks up the item, near
  the demo vehicle it enters the vehicle, and inside the demo vehicle it runs
  the engine action. The script leases `E` only while a valid context exists, so
  normal GTA behavior is left alone outside those contexts.
- Inside the demo vehicle, `H` toggles the hood/bonnet, `J` toggles the
  trunk/boot, `K` opens/closes the physical car doors, and `L` locks/unlocks the
  vehicle doors. These are also short-lived capture leases and are released when
  the player leaves the demo vehicle.

Target demo commands:

- `/targetinfo`: show client target/UI feature support
- `/targetveh`: spawn a server-driven target vehicle
- `/targetclear`: remove the target vehicle and clear the active target context

Target demo flow:

- Stand near the `/targetveh` vehicle. The client receives a short-lived target
  context and draws the center eye indicator through the client-side ImGui
  overlay.
- Press `ALT` once to open target mode. Mouse and movement input are taken by
  the ImGui overlay, and GTA camera movement is suppressed while the menu is
  open.
- Click an option. The client sends only `targetid + optionid`; the component
  validates that the target context is still active before calling Pawn.

## Helper Scripts

- `start_openmpplus.cmd` starts an unpacked open.mp server folder from the current directory.
- `install_sampp_client_admin.cmd` copies the safe ASI to the default GTA San Andreas install path. Run it as Administrator when GTA is installed under `Program Files (x86)`.
- `client-package/README.txt` is a short client-side install note for packaging or handoff.

## PAWN API

Existing SA-MP+ natives are still declared in `Build/sampp.inc`. The currently verified safe subset is HUD toggling and keybind callbacks.

Keybind example for global hotkeys:

```pawn
public OnPlayerSAMPPJoin(playerid, bool:has_plugin)
{
    if (has_plugin)
    {
        SAMPP_BindKey(playerid, SAMPP_KEY_E, SAMPP_KEY_EVENT_DOWN, "interact");
    }
    return 1;
}

public OnPlayerSAMPPKey(playerid, keyid, keystate, action[])
{
    if (keystate == SAMPP_KEY_STATE_DOWN && !strcmp(action, "interact", true))
    {
        SendClientMessage(playerid, -1, "Interact key pressed.");
    }
    return 1;
}
```

Keybind API:

- `SAMPP_BindKey(playerid, key, event_mask = SAMPP_KEY_EVENT_DOWN, const action[] = "")`
- `SAMPP_UnbindKey(playerid, key)`
- `SAMPP_ClearKeyBinds(playerid)`
- `OnPlayerSAMPPKey(playerid, keyid, keystate, action[])`

Contextual input capture API:

- `SAMPP_BeginKeyCapture(playerid, key, event_mask, priority, ttl_ms, flags, const action[])`
- `SAMPP_EndKeyCapture(playerid, key, const action[] = "")`
- `SAMPP_ClearKeyCaptures(playerid)`
- `SAMPP_CaptureKeyNearPoint(...)`
- `SAMPP_CaptureKeyInAnyVehicle(...)`
- `SAMPP_CaptureKeyInVehicle(...)`
- `SAMPP_CaptureKeyNearVehicle(...)`

Use `SAMPP_BeginKeyCapture` for interaction keys that conflict with GTA controls.
The lease is short-lived and must be renewed while the context is valid. If the
context disappears, the lease expires and the GTA default key behavior resumes.
Higher `priority` wins when multiple systems lease the same key, so a vehicle
engine action can override a nearby pickup while the player is in a vehicle.
`SAMPP_CAPTURE_DEFAULT_FLAGS` consumes keyboard input and temporarily blocks
GTA's weapon-switch action as a second safety layer for keys such as `E`.

Example:

```pawn
public OnPlayerUpdate(playerid)
{
    SAMPP_CaptureKeyNearPoint(
        playerid,
        SAMPP_KEY_E,
        x, y, z,
        2.0,
        "item_pickup",
        SAMPP_CAPTURE_PRIORITY_ITEM,
        SAMPP_CAPTURE_LEASE_DEFAULT_MS,
        SAMPP_CAPTURE_DEFAULT_FLAGS
    );
    return 1;
}
```

Client capability API:

- `SAMPP_HasFeature(playerid, SAMPP_FEATURE_*)`
- `SAMPP_GetClientFeatureFlags(playerid)`
- `SAMPP_GetClientCapabilities(playerid)`
- `SAMPP_GetClientVersion(playerid, &major, &minor, &patch)`
- `SAMPP_GetClientHash(playerid, dest[], size = sizeof dest)`
- `SAMPP_IsLauncherVerified(playerid)`
- `SAMPP_FEATURE_TARGET` indicates support for the client-side ImGui target
  overlay/menu feature.

Target UI API:

- `SAMPP_TargetBegin(playerid, targetid, const title[], ttl_ms = 500, flags = 0)`
- `SAMPP_TargetAddOption(playerid, targetid, optionid, const label[], const icon[] = "", bool:enabled = true)`
- `SAMPP_TargetCommit(playerid, targetid)`
- `SAMPP_TargetClear(playerid)`
- `OnPlayerOMPPlusTargetMode(playerid, targetid, bool:opened)`
- `OnPlayerOMPPlusTargetSelect(playerid, targetid, optionid)`

The native HELLO handshake now reports the client version, supported feature
flags, a hash of the loaded ASI, and a launcher verification flag. This is for
compatibility and feature gating. `SAMPP_IsLauncherVerified` is currently false
unless a future signed launcher token flow is added; do not treat it as
anti-cheat proof.

Example:

```pawn
if (SAMPP_HasFeature(playerid, SAMPP_FEATURE_KEYCAPTURE))
{
    SAMPP_BeginKeyCapture(playerid, SAMPP_KEY_E, SAMPP_KEY_EVENT_DOWN,
        SAMPP_CAPTURE_PRIORITY_ITEM, SAMPP_CAPTURE_LEASE_DEFAULT_MS,
        SAMPP_CAPTURE_DEFAULT_FLAGS, "item_pickup");
}
```

## Notes

This is not a full SA-MP+ feature-complete port yet. Older full-hook client behavior is intentionally kept out of the default safe ASI until each feature is isolated and tested against open.mp.

See `OPENMP_PORT.md` for porting scope, risk tiers, and remaining work.
