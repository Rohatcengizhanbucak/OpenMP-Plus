# SA-MP+ open.mp Port Notes

This branch starts by keeping SA-MP+ as a legacy SA-MP plugin, then hardening the server side for open.mp.

## Current Scope

- Server plugin reads `config.json` first and falls back to `server.cfg`.
- open.mp keys are supported:
  - `network.port`
  - `network.bind`
  - `max_players`
- Legacy SA-MP keys are still supported:
  - `port`
  - `bind`
  - `maxplayers`
- The SA-MP+ side-channel still listens on `server_port + 1`.
- Pawn native calls that require an SA-MP+ client now fail safely when the player is not connected to the side-channel.
- The safe client `.asi` connects to the side-channel without Direct3D/Input hooks.
- The safe client currently enables only the HUD component toggle RPC.
- The safe client supports limited keybind callbacks using WinAPI keyboard polling.

## Smoke Tests

Tested on GTA SA 1.0 US with open.mp 0.3DL client:

- `/sampp` verifies `IsUsingSAMPP=1` and side-channel handshake.
- `/sampphud` and `/samppmoney` toggle the money HUD.
- `/samppammo` toggles the ammo HUD.
- `/samppweapon` toggles the weapon icon.
- `/sampphealth` toggles the health bar.
- `/samppbreath` toggles the breath bar.
- `/sampparmour` and `/sampparmor` toggle the armour bar.
- `/samppmap` toggles the minimap.
- `/samppcrosshair` toggles the crosshair.
- `/samppall` toggles all supported HUD components together.
- `/samppkeys` registers smoke-test keybinds.
- F2 triggers the help action through `OnPlayerSAMPPKey`.
- B triggers the money HUD action through `OnPlayerSAMPPKey`.

Live verified so far:

- Side-channel handshake.
- Money HUD toggle.
- Ammo HUD toggle.
- Weapon icon toggle.
- Minimap toggle.
- Keybind callback: `B` returned `key=66 state=1 action=money` and toggled money HUD.
- Keybind callback: `F2` returned `key=113 state=1 action=help` and displayed help.

## Porting Order

1. Keep the side-channel, Pawn natives, and client bootstrap stable.
2. Move low-risk direct memory writes into safe mode one RPC group at a time.
3. Add explicit smoke commands for each newly enabled RPC before broad use.
4. Reintroduce hooks only when a feature cannot work through a direct RPC handler.
5. Keep the original full hook client behind an explicit unsafe/full opt-in.

Risk tiers:

- Low: HUD component toggles, simple stored player state, read-only native checks.
- Medium: HUD colours, radio/wave/game-speed style direct memory writes, keybind polling callbacks.
- High: resolution callbacks, pause menu, mouse/radio/stunt callbacks, D3D/Input proxy hooks.
- Highest: checkpoint internals, player action blocking, weapon/reload patches, cross-version GTA addresses.

## Build

Windows, for `open.mp-win-x86`:

```bat
cmake -S . -B Build\openmp-win32 -A Win32
cmake --build Build\openmp-win32 --config Release --target sampp_server
```

Linux, for `open.mp-linux-x86`:

```sh
cmake -S . -B Build/openmp-linux -DCMAKE_BUILD_TYPE=Release
cmake --build Build/openmp-linux --target sampp_server
```

Linux builds require a 32-bit toolchain and multilib C/C++ runtime.

## open.mp Installation

1. Put `sampp_server.dll` in the server `plugins` directory.
2. Add the plugin to `config.json`:

```json
"pawn": {
    "legacy_plugins": [
        "sampp_server"
    ]
}
```

3. Copy `Build/sampp.inc` to `qawno/include`.
4. Install `sampp_client.asi` on clients that should receive SA-MP+ features.

Linux server builds are not shipped yet for this port. The stale archived `.so` artifact was removed until it can be rebuilt and tested.

## Known Remaining Work

- Validate plugin load order with `Pawn.dll` and legacy plugin loading.
- Keep the safe client as the default shipped ASI; the archived client binary crashes during the first GTA SA 1.0 US load test because it applies full hooks immediately.
- Modernize or replace the full client `.asi` hooks. The client code still depends on hardcoded GTA:SA/SA-MP addresses.
- Bring additional RPCs into safe mode one group at a time, starting with low-risk direct memory patches.
- Restore resolution reporting with a narrowly scoped hook instead of the original full hook bundle.
- Revisit the side-channel player matching logic for multiple players behind the same public IP.
