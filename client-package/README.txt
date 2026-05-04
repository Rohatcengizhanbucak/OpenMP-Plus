OpenMP-Plus client package
==========================

sampp_client.asi is the current safe client build.
It opens the SA-MP+ side-channel connection and does not install Direct3D or
DirectInput hooks.

Enabled safe features:
- Side-channel handshake.
- Limited HUD component toggle RPCs.
- Limited keybind callbacks through WinAPI keyboard polling.

The keybind implementation imports USER32.dll for GetAsyncKeyState, but it does
not use DirectInput hooks.

Install:
1. Copy Build\Release\sampp_client.asi next to gta_sa.exe.
2. Start the game with an ASI loader.
3. Join the server and use /sampp.
4. Use /sampphelp to list smoke-test commands.

Smoke-test keys:
- F2 lists OpenMP-Plus smoke commands.
- B toggles the money HUD.
