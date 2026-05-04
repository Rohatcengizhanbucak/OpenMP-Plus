#include <open.mp>
#include <sampp>

#define SAMPP_TEST_COLOUR 0xA8D8FFFF
#define SAMPP_WARN_COLOUR 0xFFB86CFF
#define SAMPP_HUD_COMPONENTS (HUD_COMPONENT_MONEY + 1)

static bool:gSmokeHudState[MAX_PLAYERS][SAMPP_HUD_COMPONENTS];

stock ResetSmokeHudState(playerid)
{
	for (new component = HUD_COMPONENT_ALL; component <= HUD_COMPONENT_MONEY; component++)
	{
		gSmokeHudState[playerid][component] = true;
	}
}

stock SendSmokeHelp(playerid)
{
	SendClientMessage(playerid, SAMPP_TEST_COLOUR, "[SA-MP+] HUD tests: /samppmoney /samppammo /samppweapon /sampphealth");
	SendClientMessage(playerid, SAMPP_TEST_COLOUR, "[SA-MP+] More HUD tests: /samppbreath /sampparmour /samppmap /samppcrosshair /samppall");
	SendClientMessage(playerid, SAMPP_TEST_COLOUR, "[SA-MP+] Keybind smoke: F2 = help, B = toggle money HUD. Use /samppkeys to rebind.");
	return 1;
}

stock bool:RequireSAMPPClient(playerid)
{
	if (!IsUsingSAMPP(playerid))
	{
		SendClientMessage(playerid, SAMPP_WARN_COLOUR, "[SA-MP+] Client plugin not detected.");
		return false;
	}

	return true;
}

stock BindSmokeKeys(playerid)
{
	if (!RequireSAMPPClient(playerid))
	{
		return 1;
	}

	SAMPP_ClearKeyBinds(playerid);
	SAMPP_BindKey(playerid, SAMPP_KEY_F2, SAMPP_KEY_EVENT_DOWN, "help");
	SAMPP_BindKey(playerid, SAMPP_KEY_B, SAMPP_KEY_EVENT_DOWN, "money");

	SendClientMessage(playerid, SAMPP_TEST_COLOUR, "[SA-MP+] Keybinds registered: F2 help, B money HUD.");
	return 1;
}

stock ToggleSmokeHudComponent(playerid, component, const label[])
{
	if (!RequireSAMPPClient(playerid))
	{
		return 1;
	}

	gSmokeHudState[playerid][component] = !gSmokeHudState[playerid][component];
	ToggleHUDComponentForPlayer(playerid, component, gSmokeHudState[playerid][component]);

	new message[128];
	format(message, sizeof message, "[SA-MP+] HUD %s %s via SA-MP+ RPC.", label, gSmokeHudState[playerid][component] ? ("shown") : ("hidden"));
	SendClientMessage(playerid, SAMPP_TEST_COLOUR, message);
	return 1;
}

stock ToggleAllSmokeHud(playerid)
{
	if (!RequireSAMPPClient(playerid))
	{
		return 1;
	}

	gSmokeHudState[playerid][HUD_COMPONENT_ALL] = !gSmokeHudState[playerid][HUD_COMPONENT_ALL];

	for (new component = HUD_COMPONENT_AMMO; component <= HUD_COMPONENT_MONEY; component++)
	{
		gSmokeHudState[playerid][component] = gSmokeHudState[playerid][HUD_COMPONENT_ALL];
	}

	ToggleHUDComponentForPlayer(playerid, HUD_COMPONENT_ALL, gSmokeHudState[playerid][HUD_COMPONENT_ALL]);

	new message[128];
	format(message, sizeof message, "[SA-MP+] HUD all components %s via SA-MP+ RPC.", gSmokeHudState[playerid][HUD_COMPONENT_ALL] ? ("shown") : ("hidden"));
	SendClientMessage(playerid, SAMPP_TEST_COLOUR, message);
	return 1;
}

public OnFilterScriptInit()
{
	print("[sampp_smoketest] loaded. Use /sampp and /sampphelp in-game.");
	return 1;
}

public OnFilterScriptExit()
{
	print("[sampp_smoketest] unloaded.");
	return 1;
}

public OnPlayerConnect(playerid)
{
	ResetSmokeHudState(playerid);
	SendClientMessage(playerid, SAMPP_TEST_COLOUR, "[SA-MP+] Smoke test loaded. Use /sampp after spawning, then /sampphelp.");
	return 1;
}

public OnPlayerSAMPPJoin(playerid, bool:has_plugin)
{
	if (has_plugin)
	{
		SendClientMessage(playerid, SAMPP_TEST_COLOUR, "[SA-MP+] Client side-channel handshake OK.");
		BindSmokeKeys(playerid);
	}
	else
	{
		SendClientMessage(playerid, SAMPP_WARN_COLOUR, "[SA-MP+] Client plugin not detected yet.");
	}
	return 1;
}

public OnPlayerResolutionChange(playerid, X, Y)
{
	new message[96];
	format(message, sizeof message, "[SA-MP+] Resolution callback: %dx%d", X, Y);
	SendClientMessage(playerid, SAMPP_TEST_COLOUR, message);
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (!strcmp(cmdtext, "/sampp", true))
	{
		new bool:connected = bool:IsUsingSAMPP(playerid);
		new width, height, message[144];

		format(message, sizeof message, "[SA-MP+] Native check: IsUsingSAMPP=%d", connected);
		SendClientMessage(playerid, connected ? SAMPP_TEST_COLOUR : SAMPP_WARN_COLOUR, message);

		if (!connected)
		{
			SendClientMessage(playerid, SAMPP_WARN_COLOUR, "[SA-MP+] Install sampp_client.asi and reconnect to test RPCs.");
			return 1;
		}

		GetPlayerResolution(playerid, width, height);
		format(message, sizeof message, "[SA-MP+] Last known resolution: %dx%d", width, height);
		SendClientMessage(playerid, SAMPP_TEST_COLOUR, message);

		SendClientMessage(playerid, SAMPP_TEST_COLOUR, "[SA-MP+] Handshake OK. Use /sampphelp for HUD RPC tests.");
		return 1;
	}

	if (!strcmp(cmdtext, "/sampphelp", true) || !strcmp(cmdtext, "/samppcommands", true))
	{
		return SendSmokeHelp(playerid);
	}

	if (!strcmp(cmdtext, "/samppkeys", true) || !strcmp(cmdtext, "/samppbinds", true))
	{
		return BindSmokeKeys(playerid);
	}

	if (!strcmp(cmdtext, "/sampphud", true) || !strcmp(cmdtext, "/samppmoney", true))
	{
		return ToggleSmokeHudComponent(playerid, HUD_COMPONENT_MONEY, "money");
	}

	if (!strcmp(cmdtext, "/samppammo", true))
	{
		return ToggleSmokeHudComponent(playerid, HUD_COMPONENT_AMMO, "ammo");
	}

	if (!strcmp(cmdtext, "/samppweapon", true))
	{
		return ToggleSmokeHudComponent(playerid, HUD_COMPONENT_WEAPON, "weapon");
	}

	if (!strcmp(cmdtext, "/sampphealth", true))
	{
		return ToggleSmokeHudComponent(playerid, HUD_COMPONENT_HEALTH, "health");
	}

	if (!strcmp(cmdtext, "/samppbreath", true))
	{
		return ToggleSmokeHudComponent(playerid, HUD_COMPONENT_BREATH, "breath");
	}

	if (!strcmp(cmdtext, "/sampparmour", true) || !strcmp(cmdtext, "/sampparmor", true))
	{
		return ToggleSmokeHudComponent(playerid, HUD_COMPONENT_ARMOUR, "armour");
	}

	if (!strcmp(cmdtext, "/samppmap", true))
	{
		return ToggleSmokeHudComponent(playerid, HUD_COMPONENT_MINIMAP, "minimap");
	}

	if (!strcmp(cmdtext, "/samppcrosshair", true))
	{
		return ToggleSmokeHudComponent(playerid, HUD_COMPONENT_CROSSHAIR, "crosshair");
	}

	if (!strcmp(cmdtext, "/samppall", true))
	{
		return ToggleAllSmokeHud(playerid);
	}

	return 0;
}

public OnPlayerSAMPPKey(playerid, keyid, keystate, action[])
{
	new message[144];
	format(message, sizeof message, "[SA-MP+] Key callback: key=%d state=%d action=%s", keyid, keystate, action);
	SendClientMessage(playerid, SAMPP_TEST_COLOUR, message);

	if (keystate != SAMPP_KEY_STATE_DOWN)
	{
		return 1;
	}

	if (!strcmp(action, "help", true))
	{
		return SendSmokeHelp(playerid);
	}

	if (!strcmp(action, "money", true))
	{
		return ToggleSmokeHudComponent(playerid, HUD_COMPONENT_MONEY, "money");
	}

	return 1;
}
