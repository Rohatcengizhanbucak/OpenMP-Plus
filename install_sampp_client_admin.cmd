@echo off
setlocal
cd /d "%~dp0"

set "SOURCE=%CD%\Build\Release\sampp_client.asi"
set "GTA_DIR=%~1"
if "%GTA_DIR%"=="" set "GTA_DIR=C:\Program Files (x86)\Rockstar Games\GTA San Andreas"
set "TARGET=%GTA_DIR%\sampp_client.asi"

if not exist "%SOURCE%" (
	echo [openmpplus] Source ASI was not found:
	echo [openmpplus] %SOURCE%
	pause
	exit /b 1
)

echo [openmpplus] Installing OpenMP-Plus client ASI
echo [openmpplus] Source: %SOURCE%
echo [openmpplus] Target: %TARGET%
echo [openmpplus] This helper copies only sampp_client.asi.
echo [openmpplus] Install an ASI loader separately if this GTA folder does not already load ASI plugins.
echo.

copy /Y "%SOURCE%" "%TARGET%"
if errorlevel 1 (
	echo.
	echo [openmpplus] Install failed. Run this file as Administrator, pass your GTA folder as the first argument, or copy the ASI manually.
	pause
	exit /b 1
)

echo.
echo [openmpplus] Installed successfully.
echo [openmpplus] Also make sure the server is running the matching components\omp-plus.dll.
pause
