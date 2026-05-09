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
```

Copy the `.amx` files you want to test to your open.mp server `filterscripts` directory, then add them to your open.mp config:

```json
"pawn": {
    "side_scripts": [
        "filterscripts/sampp_smoketest",
        "filterscripts/sampp_itemdemo",
        "filterscripts/sampp_menudemo"
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

Menu demo commands:

- `/menutoggle` or `/menu`: open/close the TextDraw menu without the keybind
- `/menukeys`: rebind `M` through SA-MP+
- `/menuhelp`: list menu-demo commands

Menu demo keybind:

- `M`: open/close a player TextDraw menu through `OnPlayerSAMPPKey`

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
