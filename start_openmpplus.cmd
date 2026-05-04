@echo off
setlocal
cd /d "%~dp0"

if not exist "%CD%\omp-server.exe" (
	echo [openmpplus] omp-server.exe was not found in:
	echo [openmpplus] %CD%
	echo.
	echo [openmpplus] Run this helper from an unpacked open.mp server directory.
	pause
	exit /b 1
)

echo [openmpplus] Stopping old omp-server.exe processes...
taskkill /IM omp-server.exe /F >nul 2>nul

echo [openmpplus] Starting server: %CD%\omp-server.exe
echo [openmpplus] Game address: 127.0.0.1:7777
echo [openmpplus] SA-MP+ side-channel port: 7778
echo.

omp-server.exe

echo.
echo [openmpplus] Server closed. Press any key to exit.
pause >nul
