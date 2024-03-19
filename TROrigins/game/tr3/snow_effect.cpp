#include "pch.h"
#include "snow_effect.h"
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

// item->ocb = snowflakes spawning range.

#define MAX_MINI_WEATHER 512
static SNOWFLAKE snowflakes_2[MAX_MINI_WEATHER];

static int GetFreeSnowflake()
{
	for (int i = 0; i < MAX_MINI_WEATHER; i++)
	{
		auto* snow = &snowflakes_2[i];
		if (!snow->on)
			return i;
	}
	return -1;
}

static void UpdateSnowRoom(SNOWFLAKE* snow, long height)
{
	short new_room = snow->room_number;
	GetFloor(snow->x, snow->y + height, snow->z, &new_room);
	if (new_room != snow->room_number)
		snow->room_number = new_room;
}

static bool IsSnowBlocked(SNOWFLAKE* snow)
{
	auto current_room = snow->room_number;
	auto x = snow->x;
	auto y = snow->y;
	auto z = snow->z;
	auto* floor = GetFloor(x, y, z, &current_room);
	auto* r = &room[current_room];
	if (r->flags & ROOM_UNDERWATER)
		return true;
	auto h = GetHeight(floor, x, y, z);
	if (h == NO_HEIGHT || y > h)
		return true;
	auto c = GetCeiling(floor, x, y, z);
	if (y < c)
		return true;
	return false;
}
static bool CreateNewSnowflake(ITEM_INFO* item)
{
	int snowID = GetFreeSnowflake();
	if (snowID == -1)
		return false;
	auto* snow = &snowflakes_2[snowID];
	auto rad = GetRandomDraw() & (item->ocb != 0 ? item->ocb : 0xFFF);
	auto angle = GetRandomDraw() & 0x1FFE;
	snow->x = item->pos.x_pos + ((rad * rcossin_tbl[angle]) >> 12);
	snow->y = item->pos.y_pos - (GetRandomDraw() & 0xFF);
	snow->z = item->pos.z_pos + ((rad * rcossin_tbl[angle + 1]) >> 12);
	snow->room_number = item->room_number;
	if (IsSnowBlocked(snow)) // NOTE: it will update the room_number !
		return false;
	snow->stopped = FALSE;
	snow->on = TRUE;
	snow->size = (GetRandomDraw() & 12); // NOTE: will clamp to 4 when drawing if the value is less than 4
	snow->xv = (GetRandomDraw() & 7) - 4;
	snow->yv = (GetRandomDraw() % 24 + 8) << 3;
	snow->zv = (GetRandomDraw() & 7) - 4;
	snow->life = 1024 - (snow->yv << 1);
	snow->color = UCHAR_MAX;
	return true;
}

void ControlMiniSnowEffect(short item_number)
{
	auto* item = &items[item_number];
	for (int i = 0; i < MAX_MINI_WEATHER; i++)
	{
		CreateNewSnowflake(item);
		auto* snow = &snowflakes_2[i];
		auto ox = snow->x;
		auto oy = snow->y;
		auto oz = snow->z;

		if (!snow->stopped)
		{
			snow->x += snow->xv;
			snow->y += (snow->yv & 0xFF) >> 4;
			snow->z += snow->zv;
			UpdateSnowRoom(snow, 16);
			if (IsSnowBlocked(snow))
			{
				snow->stopped = TRUE;
				snow->x = ox;
				snow->y = oy;
				snow->z = oz;
				if (snow->yv > 16)
					snow->yv = 0;
			}
		}

		if (snow->life <= 0)
		{
			snow->on = FALSE;
			continue;
		}

		snow->life--;
		if (snow->life <= 0)
			snow->life = 0;
		if (snow->stopped)
		{
			snow->life = 1;
			snow->color--;
			if (snow->color <= 0)
				snow->color = 0;
			if (snow->color == 0)
				snow->on = FALSE;
		}

		if ((snow->yv & 7) != 7)
			snow->yv++;
	}
}

void DrawMiniSnowEffect(ITEM_INFO* item)
{
	PHDTEXTURESTRUCT tex = {};
	PHD_VECTOR pos = {};
	PHD_VBUF v[4] = {};
	bool oldBlueEffect = bBlueEffect;
	bBlueEffect = false;
#if (DIRECT3D_VERSION >= 0x900)
	auto* dm = &App.lpDeviceInfo->D3DInfo[App.lpDXConfig->nD3D].DisplayMode[App.lpDXConfig->nVMode];
#else
	auto* dm = &App.lpDeviceInfo->DDInfo[App.lpDXConfig->nDD].D3DInfo[App.lpDXConfig->nD3D].DisplayMode[App.lpDXConfig->nVMode];
#endif
	auto* sprite = &phdspriteinfo[objects[DEFAULT_SPRITES].mesh_index + ST_SNOWFLAKE];
	auto u1 = (sprite->offset << 8) & 0xFF00;
	auto v1 = sprite->offset & 0xFF00;
	auto u2 = ushort(u1 + sprite->width - App.nUVAdd);
	auto v2 = ushort(v1 + sprite->height - App.nUVAdd);
	u1 += (ushort)App.nUVAdd;
	v1 += (ushort)App.nUVAdd;

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);
	for (int i = 0; i < MAX_MINI_WEATHER; i++)
	{
		auto* snow = &snowflakes_2[i];
		if (!snow->on)
			continue;
		// If the snow is no more near the camera just don't draw it.
		if (abs(CamPos.x - snow->x) > 10240 || abs(CamPos.z - snow->z) > 10240)
			continue;
		auto tx = snow->x - item->pos.x_pos;
		auto ty = snow->y - item->pos.y_pos;
		auto tz = snow->z - item->pos.z_pos;
		pos.x = tx * phd_mxptr[M00] + ty * phd_mxptr[M01] + tz * phd_mxptr[M02] + phd_mxptr[M03];
		pos.y = tx * phd_mxptr[M10] + ty * phd_mxptr[M11] + tz * phd_mxptr[M12] + phd_mxptr[M13];
		pos.z = tx * phd_mxptr[M20] + ty * phd_mxptr[M21] + tz * phd_mxptr[M22] + phd_mxptr[M23];
		auto zv = f_persp / (float)pos.z;
		pos.x = long(float(pos.x * zv + f_centerx));
		pos.y = long(float(pos.y * zv + f_centery));
		if (pos.x < 0 || pos.x > dm->w || pos.y < 0 || pos.y > dm->h)
			continue;
		long size = phd_persp * snow->size / (pos.z >> 16);
		if (size < 4)
			size = 4;
		else if (size > 16)
			size = 16;
		size = (size * 0x2AAB) >> 15; // this scales it down to about a third of the size
		size = GetFixedScale(size)<<1;

		auto c = RGB_MAKE(snow->color, snow->color, snow->color);
		setXYZ4(v, // this drawing is based on the center of the texture.
			pos.x - size, pos.y - size, pos.z, c, // up-left
			pos.x + size, pos.y - size, pos.z, c, // up-right
			pos.x + size, pos.y + size, pos.z, c, // down-right
			pos.x - size, pos.y + size, pos.z, c, // down-left
			true
		);
		tex.drawtype = 2;
		tex.tpage = sprite->tpage;
		tex.u1 = u1;
		tex.v1 = v1;
		tex.u2 = u2;
		tex.v2 = v1;
		tex.u3 = u2;
		tex.v3 = v2;
		tex.u4 = u1;
		tex.v4 = v2;
		HWI_InsertGT4_Poly(&v[0], &v[1], &v[2], &v[3], &tex, MID_SORT, 0);
	}
	phd_PopMatrix();

	bBlueEffect = oldBlueEffect;
}