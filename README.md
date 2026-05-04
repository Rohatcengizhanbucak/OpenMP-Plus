# OpenMP-Plus

OpenMP-Plus is an open.mp-focused port and modernization branch of the archived SA-MP+ project.

The current goal is conservative: keep the original SA-MP+ side-channel model, make the server plugin load cleanly under open.mp, and re-enable client features one safe RPC group at a time.

## Current Status

- Windows open.mp legacy plugin: `Build/Release/sampp_server.dll`
- Safe client ASI: `Build/Release/sampp_client.asi`
- PAWN include: `Build/sampp.inc`
- Side-channel port: `server_port + 1`, for example `7778` when the game server uses `7777`
- Safe client mode avoids Direct3D and DirectInput hooks
- Verified on GTA SA 1.0 US with an open.mp 0.3DL client

The shipped Windows binaries are current. Linux binaries are not shipped yet for this port because the old `sampp_server.so` artifact was stale and has been removed until it can be rebuilt and tested.

## Verified Features

- open.mp `config.json` support with fallback to legacy `server.cfg`
- `IsUsingSAMPP(playerid)` side-channel detection
- HUD component toggle RPCs
- Safe keybind callbacks using WinAPI keyboard polling
- `OnPlayerSAMPPKey(playerid, keyid, keystate, action[])`

Live smoke tests confirmed:

- `/sampp` reports `IsUsingSAMPP=1`
- `/samppmoney` toggles money HUD
- `/samppammo` toggles ammo HUD
- `/samppweapon` toggles the weapon icon
- `/samppmap` toggles minimap
- `B` triggers `action=money`
- `F2` triggers `action=help`

## Installation

### Server

1. Copy `Build/Release/sampp_server.dll` to your open.mp server `plugins` directory.
2. Copy `Build/sampp.inc` to your PAWN include directory, for example `qawno/include`.
3. Add the plugin to `config.json`:

```json
"pawn": {
    "legacy_plugins": [
        "sampp_server"
    ]
}
```

4. Add your gamemode or filterscript as usual.

The plugin reads these open.mp keys:

- `network.port`
- `network.bind`
- `max_players`

It also keeps legacy fallback support for:

- `port`
- `bind`
- `maxplayers`

### Client

1. Install an ASI loader for GTA San Andreas if you do not already have one.
2. Copy `Build/Release/sampp_client.asi` next to `gta_sa.exe`.
3. Join the open.mp server.
4. Use `/sampp` in-game to confirm the side-channel handshake.

No installer is currently shipped. The old SA-MP+ installer was removed because it targeted the archived project and could install unsafe or outdated client files.

## Smoke Test

Example smoke-test files are provided in:

```text
examples/filterscripts/sampp_smoketest.pwn
examples/filterscripts/sampp_smoketest.amx
```

Copy `examples/filterscripts/sampp_smoketest.amx` to your open.mp server `filterscripts` directory, then add it to your open.mp config:

```json
"pawn": {
    "side_scripts": [
        "filterscripts/sampp_smoketest"
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

## Helper Scripts

- `start_openmpplus.cmd` starts an unpacked open.mp server folder from the current directory.
- `install_sampp_client_admin.cmd` copies the safe ASI to the default GTA San Andreas install path. Run it as Administrator when GTA is installed under `Program Files (x86)`.
- `client-package/README.txt` is a short client-side install note for packaging or handoff.

## PAWN API

Existing SA-MP+ natives are still declared in `Build/sampp.inc`. The currently verified safe subset is HUD toggling and keybind callbacks.

Keybind example:

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

## Notes

This is not a full SA-MP+ feature-complete port yet. Older full-hook client behavior is intentionally kept out of the default safe ASI until each feature is isolated and tested against open.mp.

See `OPENMP_PORT.md` for porting scope, risk tiers, and remaining work.
