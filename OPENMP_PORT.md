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

1. Put `sampp_server.dll` or `sampp_server.so` in the server `plugins` directory.
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

## Known Remaining Work

- Build and test a real Win32 DLL against the current open.mp package.
- Validate plugin load order with `Pawn.dll` and legacy plugin loading.
- Build the safe client `.asi` from source. The archived client binary crashes during the first GTA SA 1.0 US load test because it applies full hooks immediately.
- Modernize or replace the full client `.asi` hooks. The client code still depends on hardcoded GTA:SA/SA-MP addresses.
- Revisit the side-channel player matching logic for multiple players behind the same public IP.
