#include <SAMP+/client/Client.h>
#include <SAMP+/client/CCmdlineParams.h>
#include <SAMP+/client/CKeyBinds.h>
#ifndef SAMPP_SAFE_CLIENT
#include <SAMP+/client/CHooks.h>
#endif
#include <SAMP+/client/Network.h>

static HANDLE g_hBootstrapThread = NULL;
static DWORD g_dwBootstrapThreadId = 0;
static volatile bool g_bRunning = false;
static bool g_bHooksApplied = false;

static bool IsFullHookMode()
{
#ifdef SAMPP_SAFE_CLIENT
	return false;
#else
	return CCmdlineParams::ArgumentExists("sampp_full")
		|| CCmdlineParams::ArgumentExists("sampp_unsafe_hooks");
#endif
}

static DWORD WINAPI BootstrapThread(LPVOID)
{
	Sleep(2500);

	if (IsFullHookMode())
	{
#ifndef SAMPP_SAFE_CLIENT
		CLog::Write("Starting full hook mode");
		CHooks::Apply();
		g_bHooksApplied = true;
#endif
		return 0;
	}

	if (!CCmdlineParams::ArgumentExists("d")
		&& CCmdlineParams::ArgumentExists("h")
		&& CCmdlineParams::ArgumentExists("p")
		&& CCmdlineParams::ArgumentExists("sampp_legacy_sidechannel"))
	{
		const std::string& address = CCmdlineParams::GetArgumentValue("h");
		unsigned short port = (unsigned short)(atoi(CCmdlineParams::GetArgumentValue("p").c_str()) + 1);

		CLog::Write("Starting safe side-channel mode: %s:%u", address.c_str(), port);
		Network::Initialize(address, port);
		Network::Connect();
	}
	else
	{
		CLog::Write("Starting native open.mp RakClient mode");
		Network::InitializeNative();
		Network::Connect();
	}

	while (g_bRunning)
	{
		Network::Process();
		CKeyBinds::Process();
		Sleep(10);
	}

	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CLog::Initialize();
		CCmdlineParams::Process(GetCommandLine());
		g_bRunning = true;
		g_hBootstrapThread = CreateThread(NULL, 0, BootstrapThread, NULL, 0, &g_dwBootstrapThreadId);
		break;

	case DLL_PROCESS_DETACH:
		g_bRunning = false;
#ifndef SAMPP_SAFE_CLIENT
		if (g_bHooksApplied)
			CHooks::Remove();
#endif

		if (g_hBootstrapThread)
		{
			if (GetCurrentThreadId() != g_dwBootstrapThreadId)
				WaitForSingleObject(g_hBootstrapThread, 3000);
			CloseHandle(g_hBootstrapThread);
			g_hBootstrapThread = NULL;
			g_dwBootstrapThreadId = 0;
		}

		Network::Shutdown();

		CLog::Finalize();
		break;

	}
	return TRUE;
}
