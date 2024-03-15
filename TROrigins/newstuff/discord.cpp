#include "pch.h"
#include "discord.h"
#include "gameflow.h"
#include "tomb3.h"
#include "savegame.h"
#include "control.h"
#include "lara.h"
#include <discord_rpc.h>

bool RPC_title = false;
static const char* LevelPicNames[] = {
	"home",
	"jungle",
	"ruins",
	"ganges",
	"kaliya",
	"village",
	"crash",
	"madubu",
	"puna",
	"wharf",
	"train",
	"luds",
	"city",
	"nevada",
	"hsc",
	"a51",
	"antarc",
	"rx",
	"tinnos",
	"cavern",
	"hallows"
};
static const char* LevelPicNamesGold[] = {
	"home",
	"high",
	"lair",
	"cliff",
	"fish",
	"mad",
	"reunion"
};

static const char* RPC_GetLevelName()
{
	if (RPC_title)
		return "Title Screen";
	return GF_Level_Names[CurrentLevel];
}

static const char* RPC_GetTimer()
{
	long t;
	static char buf[64];
	if (RPC_title)
		return 0;
	if (CurrentLevel == LV_GYM)
	{
		t = savegame.best_assault_times[0];
		if (t == 0)
			sprintf_s(buf, "Assault Course: No Time Set!");
		else
			sprintf_s(buf, "Assault Course: %d:%2.2d.%02d", (t / 30) / 60, (t / 30) % 60, 334 * (t % 30) / 100);
	}
	else
	{
		t = savegame.timer / 30;
		sprintf_s(buf, "Time Taken: %02d:%02d:%02d", t / 3600, t / 60 % 60, t % 60);
	}
	return buf;
}

static const char* RPC_GetLevelPic()
{
	if (RPC_title)
		return "title";
	return tomb3.gold ? LevelPicNamesGold[CurrentLevel] : LevelPicNames[CurrentLevel];
}

static const char* RPC_GetHealthPic()
{
	if (lara_item == nullptr)
		return NULL;
	if (lara_item->hit_points >= 700)
		return "green";
	if (lara_item->hit_points >= 350)
		return "yellow";
	return "red";
}

static const char* RPC_GetHealthPercentage()
{
	static char buf[32];
	if (lara_item == nullptr)
		return NULL;
	sprintf_s(buf, "Health: %i%%", lara_item->hit_points / 10);
	return buf;
}

void RPC_Init()
{
	DiscordEventHandlers handlers;
	ZeroMemory(&handlers, sizeof(handlers));
	Discord_Initialize(tomb3.gold ? "1057239187859972186" : "1057182754279784508", &handlers, TRUE, NULL);
}

void RPC_Update()
{
	DiscordRichPresence RPC;
	ZeroMemory(&RPC, sizeof(RPC));
	RPC.details = RPC_GetLevelName();
	RPC.largeImageKey = RPC_GetLevelPic();
	RPC.largeImageText = RPC.details;
	RPC.smallImageKey = RPC_GetHealthPic();
	RPC.smallImageText = RPC_GetHealthPercentage();
	RPC.state = RPC_GetTimer();
	RPC.instance = TRUE;
	Discord_UpdatePresence(&RPC);
}

void RPC_close()
{
	Discord_Shutdown();
}
