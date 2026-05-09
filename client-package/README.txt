OpenMP-Plus client package
==========================

sampp_client.asi is the safe client build.
The current source defaults to the native game RakClient transport and does not
install Direct3D or DirectInput hooks. The old side-channel can still be forced
with -sampp_legacy_sidechannel while testing legacy servers.

Enabled safe features:
- Native OMP+ handshake over the existing game connection.
- Limited HUD component toggle RPCs.
- Limited keybind callbacks through WinAPI keyboard polling.

The keybind implementation imports USER32.dll for GetAsyncKeyState, but it does
not use DirectInput hooks.

Install:
1. Find the GTA San Andreas folder that actually launches the game.
2. Install an ASI loader there if ASI plugins are not already loading.
3. Copy Build\Release\sampp_client.asi next to gta_sa.exe.
4. Make sure the server is also running the matching native component:
   components\omp-plus.dll.
5. Join the server and use /sampp.
6. Use /sampphelp to list smoke-test commands.

Client-side files:
- sampp_client.asi
- ASI loader files, only if the GTA folder does not already have a working ASI
  loader
  Common loader layouts include vorbisFile.dll plus vorbisHooked.dll, dinput8.dll,
  or version.dll depending on the loader package.

Server-side files, for reference:
- components\omp-plus.dll
- qawno\include\sampp.inc when compiling scripts
- filterscripts\sampp_smoketest.amx when testing the sample filterscript

Expected /sampp result:
- IsUsingSAMPP=1

If IsUsingSAMPP stays 0, the usual causes are an old server DLL, an old client
ASI, the ASI copied into the wrong GTA folder, or a missing ASI loader.

Smoke-test keys:
- F2 lists OpenMP-Plus smoke commands.
- B toggles the money HUD.
