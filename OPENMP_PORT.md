# SA-MP+ open.mp Port Notes

This branch now contains a native open.mp component transport. The legacy SA-MP+ side-channel remains in the tree as a fallback, but the target architecture is the open.mp `INetwork` / RakServer pipeline.

## Current Scope

- Native server mode is an open.mp component loaded from `components/omp-plus.dll`.
- Native component mode uses custom RPC `220` over the existing player connection.
- Native mode does not open a second UDP port and does not create a second server RakPeer.
- Legacy fallback mode still exists in the tree as `sampp_server.dll`.
- The legacy fallback plugin reads `config.json` first and falls back to `server.cfg`.
- These open.mp keys are supported by the legacy fallback:
  - `network.port`
  - `network.bind`
  - `max_players`
- These legacy SA-MP keys are still supported by the legacy fallback:
  - `port`
  - `bind`
  - `maxplayers`
- The SA-MP+ side-channel listens on `server_port + 1` only in legacy fallback mode.
- Pawn native calls that require an SA-MP+ client now fail safely when the player has not completed the native OMP+ handshake.
- The safe client `.asi` defaults to the game's existing RakClient connection and uses custom RPC `220`.
- The legacy side-channel client path is available only with the explicit `-sampp_legacy_sidechannel` flag.
- The safe client enables HUD toggles and keybind callbacks without Direct3D/Input hooks.
- The safe client supports limited keybind callbacks using WinAPI keyboard polling.

## Smoke Tests

Smoke tests target GTA SA 1.0 US with the open.mp/SA-MP compatible client and
the native transport:

- `/sampp` verifies `IsUsingSAMPP=1` and the compatibility handshake.
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

Previously live verified in the safe feature subset:

- Native compatibility handshake.
- Money HUD toggle.
- Ammo HUD toggle.
- Weapon icon toggle.
- Minimap toggle.
- Keybind callback: `B` returned `key=66 state=1 action=money` and toggled money HUD.
- Keybind callback: `F2` returned `key=113 state=1 action=help` and displayed help.

## Porting Order

1. Keep native open.mp component transport as the default server path.
2. Keep the safe client on the existing game RakClient connection by default.
3. Move low-risk direct memory writes into safe mode one RPC group at a time.
4. Add explicit smoke commands for each newly enabled RPC before broad use.
5. Reintroduce hooks only when a feature cannot work through a direct RPC handler.
6. Keep the original full hook client behind an explicit unsafe/full opt-in.

Risk tiers:

- Low: HUD component toggles, simple stored player state, read-only native checks.
- Medium: HUD colours, radio/wave/game-speed style direct memory writes, keybind polling callbacks.
- High: resolution callbacks, pause menu, mouse/radio/stunt callbacks, D3D/Input proxy hooks.
- Highest: checkpoint internals, player action blocking, weapon/reload patches, cross-version GTA addresses.

## Build

Native component, Windows x86:

```bat
git submodule update --init --recursive
cmake -S . -B Build\openmp-native-win32 -A Win32 -D OMPPLUS_BUILD_COMPONENT=ON
cmake --build Build\openmp-native-win32 --config Release --target omp_plus_component
```

The native artifact is:

```text
Build/Release/omp-plus.dll
```

Legacy fallback plugin, Windows x86:

```bat
cmake -S . -B Build\openmp-win32 -A Win32
cmake --build Build\openmp-win32 --config Release --target sampp_server
```

Legacy fallback plugin, Linux x86:

```sh
cmake -S . -B Build/openmp-linux -DCMAKE_BUILD_TYPE=Release
cmake --build Build/openmp-linux --target sampp_server
```

Linux builds require a 32-bit toolchain and multilib C/C++ runtime.

## open.mp Installation

1. Put `omp-plus.dll` in the server `components` directory.
2. Leave the top-level `components` key absent unless your server already uses a
   full explicit component list. Copying the DLL into `components` is the
   recommended native install.
3. If your server intentionally uses an explicit top-level `components` list,
   add `omp-plus` to the complete list without removing the default components.
   Do not replace the list with only `omp-plus`, because that can prevent the
   Pawn component from loading.

Legacy fallback only: put `sampp_server.dll` in the server `plugins` directory and add it as a legacy plugin:

```json
"pawn": {
    "legacy_plugins": [
        "sampp_server"
    ]
}
```

4. Copy `Build/sampp.inc` to `qawno/include`.
5. Install `sampp_client.asi` and an ASI loader on clients that should receive SA-MP+ features. Native mode does not require opening a second UDP port.

Linux server builds are not shipped yet for this port. The stale archived `.so` artifact was removed until it can be rebuilt and tested.

## Known Remaining Work

- Expand the native component test matrix across open.mp package versions.
- Keep the safe client as the default shipped ASI; the archived client binary crashes during the first GTA SA 1.0 US load test because it applies full hooks immediately.
- Modernize or replace the full client `.asi` hooks. The client code still depends on hardcoded GTA:SA/SA-MP addresses.
- Bring additional RPCs into safe mode one group at a time, starting with low-risk direct memory patches.
- Restore resolution reporting with a narrowly scoped hook instead of the original full hook bundle.
- Keep legacy side-channel mode as fallback only, or remove it after native coverage is complete.
