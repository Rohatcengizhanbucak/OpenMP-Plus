# OpenMP-Plus Installation Guide

This guide covers the native open.mp transport. Native mode uses the existing
open.mp RakServer / player connection and does not require a second UDP port.

Install the server component and client ASI from the same build or release. Old
server DLLs paired with new ASIs, or old ASIs paired with new server DLLs, can
make the client appear as not detected.

## What Goes Where

Server files:

| File | Destination | Required | Notes |
| --- | --- | --- | --- |
| `Build/Release/omp-plus.dll` | `<openmp-server>/components/omp-plus.dll` | Yes | Native open.mp component. |
| `Build/sampp.inc` | `<openmp-server>/qawno/include/sampp.inc` | For compiling scripts | Pawn include for old SA-MP+ names and new OMP+ names. |
| `examples/filterscripts/*.amx` | `<openmp-server>/filterscripts/` | Optional | Smoke-test and demo filterscripts. |
| `Build/Release/sampp_server.dll` | `<openmp-server>/plugins/sampp_server.dll` | No | Legacy side-channel fallback only. Built only with `OMPPLUS_BUILD_LEGACY_PLUGIN=ON`; do not use for native mode. |

Client files:

| File | Destination | Required | Notes |
| --- | --- | --- | --- |
| `Build/Release/sampp_client.asi` | `<gta-sa>/sampp_client.asi` | Yes | Client extension loaded by the ASI loader. |
| ASI loader files | `<gta-sa>/` | If not already installed | Required for GTA San Andreas to load `.asi` plugins. |

Do not copy `omp-plus.dll` to the client. Do not copy `sampp_client.asi` to the
server. They are separate halves of the same protocol.

## Server Setup

1. Copy `Build/Release/omp-plus.dll` to the server `components` directory.
2. Copy `Build/sampp.inc` to `qawno/include` if you compile Pawn scripts on that
   server install.
3. Optional: copy smoke-test and demo `.amx` files to `filterscripts`.
4. Start `omp-server.exe`.

Recommended native component layout:

```text
<openmp-server>/
  omp-server.exe
  config.json
  components/
    omp-plus.dll
  qawno/
    include/
      sampp.inc
  filterscripts/
    sampp_smoketest.amx
    sampp_itemdemo.amx
    sampp_capabilitydemo.amx
    sampp_targetdemo.amx
    sampp_builddemo.amx
```

Do not add this as the only top-level component list:

```json
"components": [
    "omp-plus"
]
```

On some open.mp packages that replaces the default component set and prevents
the Pawn component from loading. The recommended install is to leave the
top-level `components` key absent and let open.mp load the directory normally.

If your server already uses a full explicit top-level `components` list, add
`omp-plus` to that full list without removing the other required components.

To load the smoke-test and capability demo filterscripts, add them under
`pawn.side_scripts`:

```json
"pawn": {
    "side_scripts": [
        "filterscripts/sampp_smoketest",
        "filterscripts/sampp_capabilitydemo",
        "filterscripts/sampp_targetdemo",
        "filterscripts/sampp_builddemo"
    ]
}
```

Keep `sampp_builddemo` as its own side script. Do not also load another
filterscript that embeds `sampp_builddemo_core`, because only one Pawn script
should own `/builddemo` and the build callbacks for a player session.

Useful in-game checks:

- `/sampp`: basic native transport and client-info smoke test.
- `/capinfo`: version, feature flags, capabilities, hash prefix, and launcher
  verification flag.
- `/capspawn`: context-priority demo for `E`; item pickup, vehicle entry, and
  in-vehicle engine action use short-lived capture leases instead of a global
  keybind.
- Inside the `/capspawn` demo vehicle, `H` toggles hood/bonnet, `J` toggles
  trunk/boot, `K` opens/closes physical doors, and `L` locks/unlocks the vehicle.
- `/targetveh`: server-driven target UI demo. Stand near the spawned vehicle,
  press `ALT` once to open the mouse menu, and select an option. The server
  validates the selected `targetid` and `optionid` before Pawn callbacks run.
- `/builddemo`: experimental server-authoritative build UI demo. The client
  renders the build menu and sends select/place/cancel requests; Pawn creates
  the real objects only after validating the request. Wall, Door Frame, Floor,
  Roof, and Door snap to validated foundation slots; middle mouse switches side
  for walls/doors, left mouse places, right mouse returns from placement to the
  menu, and right mouse in the menu or `ESC` closes.

Expected server log line:

```text
[OpenMP-Plus] Native INetwork transport loaded on RPC 220.
```

## Client Setup

1. Find the GTA San Andreas folder that your launcher actually starts. The right
   folder is the one containing the `gta_sa.exe` used by the game session.
2. Install an ASI loader if that GTA install does not already load `.asi`
   plugins.
3. Copy `Build/Release/sampp_client.asi` next to that `gta_sa.exe`.
4. Join the server and use `/sampp`.

Recommended client layout:

```text
<gta-sa>/
  gta_sa.exe
  sampp_client.asi
  <ASI loader DLLs>
```

The exact ASI loader DLL names depend on the loader package. Common layouts
include `vorbisFile.dll` plus `vorbisHooked.dll`, or a proxy such as
`dinput8.dll` or `version.dll`. If other ASI mods already load in that GTA
folder, OpenMP-Plus can use the same loader.

Expected client log lines in the GTA folder `log.txt`:

```text
ImGui target UI graphics hook enabled
ImGui target overlay hook installed through existing D3D9 device
Starting native open.mp RakClient mode
Native RakClient transport registered RPC 220
```

Expected in-game result from `/sampp`:

```text
IsUsingSAMPP=1
```

## Legacy Side-Channel Mode

Legacy mode is only for testing old servers or old behavior. It opens the old
`server_port + 1` connection and uses the legacy `sampp_server.dll` plugin.

Native mode should not require:

- Opening UDP `7778` when the game port is `7777`.
- Installing `sampp_server.dll` as a legacy plugin.
- Passing `-sampp_legacy_sidechannel` to the client.

To build the legacy plugin anyway, configure CMake explicitly:

```bat
cmake -S . -B Build\openmp-legacy-win32 -A Win32 -D OMPPLUS_BUILD_LEGACY_PLUGIN=ON
cmake --build Build\openmp-legacy-win32 --config Release --target sampp_server
```

Native transport uses custom RPC `220`. Keep that ID reserved for OpenMP-Plus
unless you intentionally change and rebuild both the component and the ASI.

Contextual key capture uses the same native transport. Install a matching
`omp-plus.dll`, `sampp_client.asi`, and `sampp.inc`; otherwise Pawn scripts may
compile with `SAMPP_BeginKeyCapture` while the client cannot consume the GTA
default key input. The default capture flags also suppress GTA weapon switching
while a capture lease is active. Close GTA before replacing `sampp_client.asi`,
because Windows locks loaded ASI files while the game is running.

The client and component must also match for capability negotiation. On connect,
the ASI reports:

- OpenMP-Plus client version.
- Supported feature flags such as HUD, keybind and key capture.
- Target UI support when the ASI can render the Dear ImGui overlay and suppress
  mouse camera input while target mode is open.
- SHA-256 hash of the loaded ASI when Windows CryptoAPI can read it.
- Launcher verification status. This is currently informational and should not
  be treated as anti-cheat proof.

Pawn scripts can gate features with:

```pawn
if (SAMPP_HasFeature(playerid, SAMPP_FEATURE_KEYCAPTURE))
{
    // safe to use contextual input capture
}
```

## Troubleshooting

`/sampp` shows `IsUsingSAMPP=0`:

- The ASI is not in the GTA folder that actually launched.
- The ASI loader is missing or not loading plugins.
- The server is still running an old `omp-plus.dll`.
- The client is still running an old `sampp_client.asi`.
- The server config disabled the Pawn component by replacing the default
  top-level `components` list with only `omp-plus`.

Server logs `Pawn component not found`:

- Remove the top-level `"components": ["omp-plus"]` shortcut.
- Let the server load its default components, or use a complete explicit
  component list that includes Pawn and every other required component.

No OpenMP-Plus log appears in `log.txt`:

- Confirm that `sampp_client.asi` is next to the launched `gta_sa.exe`.
- Confirm that the ASI loader is installed in the same GTA folder.

Native mode starts but does not complete:

- Replace both `components/omp-plus.dll` and `sampp_client.asi` from the same
  build.
- Restart the server and reconnect the client.
