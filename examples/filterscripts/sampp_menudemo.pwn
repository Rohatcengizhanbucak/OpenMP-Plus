#include <open.mp>
#include <sampp>

#define MENU_DEMO_COLOUR 0x9DEB7DFF
#define MENU_DEMO_WARN_COLOUR 0xFFB86CFF

#define MENU_TD_BACKGROUND 0
#define MENU_TD_TITLE 1
#define MENU_TD_ROW_1 2
#define MENU_TD_ROW_2 3
#define MENU_TD_ROW_3 4
#define MENU_TD_FOOTER 5
#define MENU_TD_COUNT 6

static PlayerText:gMenuTextDraw[MAX_PLAYERS][MENU_TD_COUNT];
static bool:gMenuShown[MAX_PLAYERS];

stock ResetMenuState(playerid)
{
	for (new index = 0; index < MENU_TD_COUNT; index++)
	{
		gMenuTextDraw[playerid][index] = INVALID_PLAYER_TEXT_DRAW;
	}

	gMenuShown[playerid] = false;
}

stock SetupMenuTextDraw(playerid, index, Float:x, Float:y, const text[], Float:letterWidth = 0.22, Float:letterHeight = 1.05)
{
	gMenuTextDraw[playerid][index] = CreatePlayerTextDraw(playerid, x, y, text);
	PlayerTextDrawFont(playerid, gMenuTextDraw[playerid][index], TEXT_DRAW_FONT_STANDARD);
	PlayerTextDrawLetterSize(playerid, gMenuTextDraw[playerid][index], letterWidth, letterHeight);
	PlayerTextDrawColour(playerid, gMenuTextDraw[playerid][index], 0xFFFFFFFF);
	PlayerTextDrawBackgroundColour(playerid, gMenuTextDraw[playerid][index], 0x000000AA);
	PlayerTextDrawSetOutline(playerid, gMenuTextDraw[playerid][index], 1);
	PlayerTextDrawSetShadow(playerid, gMenuTextDraw[playerid][index], 0);
	PlayerTextDrawSetProportional(playerid, gMenuTextDraw[playerid][index], true);
	PlayerTextDrawAlignment(playerid, gMenuTextDraw[playerid][index], TEXT_DRAW_ALIGN_LEFT);
}

stock CreateMenuTextDraws(playerid)
{
	if (gMenuTextDraw[playerid][MENU_TD_BACKGROUND] != INVALID_PLAYER_TEXT_DRAW)
	{
		return 1;
	}

	gMenuTextDraw[playerid][MENU_TD_BACKGROUND] = CreatePlayerTextDraw(playerid, 205.0, 112.0, "_");
	PlayerTextDrawFont(playerid, gMenuTextDraw[playerid][MENU_TD_BACKGROUND], TEXT_DRAW_FONT_STANDARD);
	PlayerTextDrawLetterSize(playerid, gMenuTextDraw[playerid][MENU_TD_BACKGROUND], 0.0, 10.0);
	PlayerTextDrawTextSize(playerid, gMenuTextDraw[playerid][MENU_TD_BACKGROUND], 435.0, 0.0);
	PlayerTextDrawUseBox(playerid, gMenuTextDraw[playerid][MENU_TD_BACKGROUND], true);
	PlayerTextDrawBoxColour(playerid, gMenuTextDraw[playerid][MENU_TD_BACKGROUND], 0x101820CC);
	PlayerTextDrawColour(playerid, gMenuTextDraw[playerid][MENU_TD_BACKGROUND], 0x00000000);
	PlayerTextDrawAlignment(playerid, gMenuTextDraw[playerid][MENU_TD_BACKGROUND], TEXT_DRAW_ALIGN_LEFT);

	SetupMenuTextDraw(playerid, MENU_TD_TITLE, 220.0, 122.0, "OPENMP-PLUS DEMO MENU", 0.34, 1.35);
	PlayerTextDrawColour(playerid, gMenuTextDraw[playerid][MENU_TD_TITLE], 0x9DEB7DFF);

	SetupMenuTextDraw(playerid, MENU_TD_ROW_1, 220.0, 152.0, "1. /itemadd creates a Water Bottle pickup");
	SetupMenuTextDraw(playerid, MENU_TD_ROW_2, 220.0, 174.0, "2. E picks up nearby demo items");
	SetupMenuTextDraw(playerid, MENU_TD_ROW_3, 220.0, 196.0, "3. F2/B still test SA-MP+ HUD RPCs");
	SetupMenuTextDraw(playerid, MENU_TD_FOOTER, 220.0, 228.0, "Press M again to close this menu", 0.20, 0.95);
	PlayerTextDrawColour(playerid, gMenuTextDraw[playerid][MENU_TD_FOOTER], 0xB8C7D9FF);
	return 1;
}

stock ShowMenu(playerid)
{
	CreateMenuTextDraws(playerid);

	for (new index = 0; index < MENU_TD_COUNT; index++)
	{
		PlayerTextDrawShow(playerid, gMenuTextDraw[playerid][index]);
	}

	gMenuShown[playerid] = true;
	SendClientMessage(playerid, MENU_DEMO_COLOUR, "[MenuDemo] TextDraw menu opened via SA-MP+ keybind.");
	return 1;
}

stock HideMenu(playerid)
{
	if (gMenuTextDraw[playerid][MENU_TD_BACKGROUND] == INVALID_PLAYER_TEXT_DRAW)
	{
		return 1;
	}

	for (new index = 0; index < MENU_TD_COUNT; index++)
	{
		PlayerTextDrawHide(playerid, gMenuTextDraw[playerid][index]);
	}

	gMenuShown[playerid] = false;
	SendClientMessage(playerid, MENU_DEMO_COLOUR, "[MenuDemo] TextDraw menu closed via SA-MP+ keybind.");
	return 1;
}

stock ToggleMenu(playerid)
{
	if (gMenuShown[playerid])
	{
		return HideMenu(playerid);
	}

	return ShowMenu(playerid);
}

stock DestroyMenuTextDraws(playerid)
{
	if (gMenuShown[playerid])
	{
		HideMenu(playerid);
	}

	for (new index = 0; index < MENU_TD_COUNT; index++)
	{
		if (gMenuTextDraw[playerid][index] != INVALID_PLAYER_TEXT_DRAW)
		{
			PlayerTextDrawDestroy(playerid, gMenuTextDraw[playerid][index]);
		}
	}

	ResetMenuState(playerid);
	return 1;
}

stock BindMenuKey(playerid)
{
	if (!IsUsingSAMPP(playerid))
	{
		SendClientMessage(playerid, MENU_DEMO_WARN_COLOUR, "[MenuDemo] SA-MP+ client plugin not detected yet.");
		return 1;
	}

	SAMPP_UnbindKey(playerid, SAMPP_KEY_M);
	SAMPP_BindKey(playerid, SAMPP_KEY_M, SAMPP_KEY_EVENT_DOWN, "menu_toggle");

	SendClientMessage(playerid, MENU_DEMO_COLOUR, "[MenuDemo] M key bound through SA-MP+. Press M to open/close the menu.");
	return 1;
}

stock SendMenuHelp(playerid)
{
	SendClientMessage(playerid, MENU_DEMO_COLOUR, "[MenuDemo] Press M to toggle the TextDraw menu through SA-MP+.");
	SendClientMessage(playerid, MENU_DEMO_COLOUR, "[MenuDemo] /menutoggle toggles it without the keybind; /menukeys rebinds M.");
	return 1;
}

public OnFilterScriptInit()
{
	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		ResetMenuState(playerid);
	}

	print("[sampp_menudemo] loaded. Press M to toggle the TextDraw menu.");
	return 1;
}

public OnFilterScriptExit()
{
	for (new playerid = 0; playerid < MAX_PLAYERS; playerid++)
	{
		DestroyMenuTextDraws(playerid);
	}

	print("[sampp_menudemo] unloaded.");
	return 1;
}

public OnPlayerConnect(playerid)
{
	ResetMenuState(playerid);
	SendClientMessage(playerid, MENU_DEMO_COLOUR, "[MenuDemo] Press M to open/close the SA-MP+ TextDraw demo menu.");
	return 1;
}

public OnPlayerDisconnect(playerid, reason)
{
	DestroyMenuTextDraws(playerid);
	return 1;
}

public OnPlayerSAMPPJoin(playerid, bool:has_plugin)
{
	if (has_plugin)
	{
		BindMenuKey(playerid);
	}
	return 1;
}

public OnPlayerCommandText(playerid, cmdtext[])
{
	if (!strcmp(cmdtext, "/menutoggle", true) || !strcmp(cmdtext, "/menu", true))
	{
		return ToggleMenu(playerid);
	}

	if (!strcmp(cmdtext, "/menukeys", true))
	{
		return BindMenuKey(playerid);
	}

	if (!strcmp(cmdtext, "/menuhelp", true))
	{
		return SendMenuHelp(playerid);
	}

	return 0;
}

public OnPlayerSAMPPKey(playerid, keyid, keystate, action[])
{
	if (keyid != SAMPP_KEY_M || keystate != SAMPP_KEY_STATE_DOWN || strcmp(action, "menu_toggle", true))
	{
		return 1;
	}

	SendClientMessage(playerid, MENU_DEMO_COLOUR, "[MenuDemo] M key callback received from SA-MP+ ASI.");
	return ToggleMenu(playerid);
}
