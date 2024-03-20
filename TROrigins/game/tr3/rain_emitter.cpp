#include "pch.h"
#include "rain_emitter.h"
#include "3d_gen.h"
#include "effect2.h"
#include "control.h"
#include "output.h"
#include "draweffects.h"
#include "file.h"
#include "init.h"
#include "game.h"
#include "setup.h"
#include "objects.h"
#include "hwinsert.h"
#include "winmain.h"
#include "items.h"
#include "camera.h"
#include "draw.h"
#include "tomb3.h"

constexpr auto MAX_MINI_WEATHER = 128;

static int GetFreeDrop(RAINDROP* droplist)
{
	for (int i = 0; i < MAX_MINI_WEATHER; i++)
	{
		auto* drop = &droplist[i];
		if (!drop->on)
			return i;
	}
	return -1;
}

static inline RAINDROP* GetDropList(ITEM_INFO* item)
{
	return static_cast<RAINDROP*>(item->data);
}

static void UpdateDropRoom(RAINDROP* drop, long height)
{
	short new_room = drop->room_number;
	GetFloor(drop->x, drop->y + height, drop->z, &new_room);
	if (new_room != drop->room_number)
		drop->room_number = new_room;
}

static bool IsDropBlocked(RAINDROP* drop)
{
	auto current_room = drop->room_number;
	auto x = drop->x;
	auto y = drop->y;
	auto z = drop->z;
	auto* floor = GetFloor(x, y, z, &current_room);
	auto* r = &room[current_room];
	if ((r->flags & ROOM_UNDERWATER) || (r->flags & ROOM_SWAMP))
		return true;
	auto h = GetHeight(floor, x, y, z);
	if (h == NO_HEIGHT || y > h)
		return true;
	auto c = GetCeiling(floor, x, y, z);
	if (y < c)
		return true;
	return false;
}
static bool CreateNewDrop(ITEM_INFO* item)
{
	auto* leaveList = GetDropList(item);
	int leaveID = GetFreeDrop(leaveList);
	if (leaveID == -1)
		return false;
	auto* drop = &leaveList[leaveID];
	auto rad = GetRandomDraw() & (item->ocb > 0 ? (item->ocb - 1) : (SECTOR(4) - 1));
	auto angle = GetRandomDraw() & 0x1FFE;
	drop->x = item->pos.x_pos + ((rad * rcossin_tbl[angle]) >> 12);
	drop->y = item->pos.y_pos - (GetRandomDraw() & 0xFF);
	drop->z = item->pos.z_pos + ((rad * rcossin_tbl[angle + 1]) >> 12);
	drop->room_number = item->room_number;
	if (IsDropBlocked(drop)) // NOTE: it will update the room_number !
		return false;
	drop->on = TRUE;
	drop->xv = (GetRandomDraw() & 7) - 4;
	if (tomb3.improved_rain)
	{
		auto tx = GetFixedScale(1);
		drop->yv = uchar((GetRandomDraw() & 7) + (tx * 8));
	}
	else
	{
		drop->yv = (GetRandomDraw() & 7) + 16;
	}
	drop->zv = (GetRandomDraw() & 7) - 4;
	drop->life = 1024 - (drop->yv << 1);
	return true;
}

void InitialiseRainEmitter(short item_number)
{
	auto* item = &items[item_number];
	item->data = (RAINDROP*)game_malloc(sizeof(RAINDROP) * MAX_MINI_WEATHER);
	auto* dropList = GetDropList(item);
	for (int i = 0; i < MAX_MINI_WEATHER; i++) // WARM it !
		ControlRainEmitter(item_number);
}

void ControlRainEmitter(short item_number)
{
	auto* item = &items[item_number];
	auto* dropList = GetDropList(item);
	for (int i = 0; i < MAX_MINI_WEATHER; i++)
	{
		CreateNewDrop(item);
		auto* drop = &dropList[i];

		drop->x += drop->xv;
		drop->y += drop->yv << 3;
		drop->z += drop->zv;

		// Does room is outside and not underwater ?
		if ((room[drop->room_number].flags & ROOM_NOT_INSIDE) && !(room[drop->room_number].flags & ROOM_UNDERWATER))
		{
			drop->x += 4 * SmokeWindX;
			drop->z += 4 * SmokeWindZ;
		}

		UpdateDropRoom(drop, 16);
		if (IsDropBlocked(drop))
		{
			drop->on = FALSE;
			continue;
		}

		auto rnd = GetRandomDraw();
		if ((rnd & 3) != 3)
		{
			drop->xv += (rnd & 3) - 1;
			if (drop->xv < -4)
				drop->xv = -4;
			else if (drop->xv > 4)
				drop->xv = 4;
		}
		rnd = (rnd >> 2) & 3;
		if (rnd != 3)
		{
			drop->zv += char(rnd - 1);
			if (drop->zv < -4)
				drop->zv = -4;
			else if (drop->zv > 4)
				drop->zv = 4;
		}

		drop->life -= 2;
		if (!drop->life)
			drop->on = FALSE;
	}
}

void DrawRainEmitter(ITEM_INFO* item)
{
	PHDTEXTURESTRUCT tex;
	PHD_VBUF v[4] = {};
	PHD_VECTOR pos;
	long* pXY;
	long* pZ;
	long XY[4];
	long Z[2];
	auto* dropList = GetDropList(item);

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	pXY = XY;
	pZ = Z;

	for (int i = 0; i < MAX_MINI_WEATHER; i++)
	{
		auto* drop = &dropList[i];
		if (!drop->on)
			continue;
		// If the drop is no more in the view distance of the camera, just don't draw it.
		if (abs(CamPos.x - drop->x) > phd_viewdist || abs(CamPos.z - drop->z) > phd_viewdist)
			continue;

		// Does room is outside and not underwater ?
		long windX = 0;
		long windZ = 0;
		if ((room[drop->room_number].flags & ROOM_NOT_INSIDE) && !(room[drop->room_number].flags & ROOM_UNDERWATER))
		{
			windX = (SmokeWindX << 2);
			windZ = (SmokeWindZ << 2);
		}
		auto tx = drop->x - item->pos.x_pos - windX;
		auto ty = drop->y - item->pos.y_pos - (drop->yv << 3);
		auto tz = drop->z - item->pos.z_pos - windZ;
		pos.x = tx * phd_mxptr[M00] + ty * phd_mxptr[M01] + tz * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = tx * phd_mxptr[M10] + ty * phd_mxptr[M11] + tz * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = tx * phd_mxptr[M20] + ty * phd_mxptr[M21] + tz * phd_mxptr[M22] + phd_mxptr[M23];
		auto zv = f_persp / (float)pos.z;
		pos.x = long(float(pos.x * zv + f_centerx));
		pos.y = long(float(pos.y * zv + f_centery));
		pXY[0] = pos.x;
		pXY[1] = pos.y;
		pZ[0] = pos.z;
		pXY += 2;
		pZ++;

		tx = drop->x - item->pos.x_pos;
		ty = drop->y - item->pos.y_pos;
		tz = drop->z - item->pos.z_pos;
		pos.x = tx * phd_mxptr[M00] + ty * phd_mxptr[M01] + tz * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = tx * phd_mxptr[M10] + ty * phd_mxptr[M11] + tz * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = tx * phd_mxptr[M20] + ty * phd_mxptr[M21] + tz * phd_mxptr[M22] + phd_mxptr[M23];
		zv = f_persp / (float)pos.z;
		pos.x = long(float(pos.x * zv + f_centerx));
		pos.y = long(float(pos.y * zv + f_centery));
		pXY[0] = pos.x;
		pXY[1] = pos.y;
		pZ[0] = pos.z;
		pXY -= 2;
		pZ--;

		auto x1 = pXY[0];
		auto y1 = pXY[1];
		auto x2 = pXY[2];
		auto y2 = pXY[3];
		auto z = pZ[0];
		if (z < 32 || x1 < phd_winxmin || x1 > phd_winxmin + phd_winxmax || y1 < phd_winymin || y1 > phd_winymin + phd_winymax)
			continue;

		bool oldBlueEffect = bBlueEffect;
		bBlueEffect = false;
		if (ClipLine(x1, y1, x2, y2, 0, 0))
		{
			if (tomb3.improved_rain)
			{
				auto rnd = GetFixedScale(1); //width

				v[0].xs = (float)x1;
				v[0].ys = (float)y1;
				v[0].color = 0;

				v[1].xs = (float)x1 + rnd;
				v[1].ys = (float)y1;
				v[1].color = 0;

				v[2].xs = (float)x2 + rnd;
				v[2].ys = (float)y2;
				v[2].color = 0x6080C0;

				v[3].xs = (float)x2;
				v[3].ys = (float)y2;
				v[3].color = v[2].color;

				for (int i = 0; i < 4; i++)
				{
					v[i].zv = (float)z;
					v[i].ooz = f_persp / (float)z * f_oneopersp;
					v[i].clip = 0;
				}

				ZeroMemory(&tex, sizeof(tex));
				tex.drawtype = 2;
				HWI_InsertGT4_Sorted(&v[0], &v[1], &v[2], &v[3], &tex, MID_SORT, 1);
			}
			else
			{
				auto oldAlpha = GlobalAlpha;
				GlobalAlpha = RGBA_MAKE(0, 0, 0, 128);
				HWI_InsertLine_Sorted(x1 - phd_winxmin, y1 - phd_winymin, x2 - phd_winxmin, y2 - phd_winymin, z, 0x20, 0x304060);
				GlobalAlpha = oldAlpha;
			}
		}
		bBlueEffect = oldBlueEffect;
	}

	phd_PopMatrix();
}
