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

static bool IsSnowBlocked(SNOWFLAKE* snow)
{
	auto* floor = GetFloor(snow->x, snow->y, snow->z, &snow->room_number);
	auto* r = &room[snow->room_number];
	if (r->flags & ROOM_UNDERWATER)
		return true;
	if (snow->y > r->maxceiling && snow->y < r->minfloor &&
		(snow->z > r->z + WALL_SIZE && snow->z < (r->x_size << WALL_SHIFT) + r->z - WALL_SIZE) &&
		(snow->x > r->x + WALL_SIZE && snow->x < (r->y_size << WALL_SHIFT) + r->x - WALL_SIZE))
	{
		auto h = GetHeight(floor, snow->x, snow->y, snow->z);
		if (h == NO_HEIGHT || snow->y > h)
			return true;
		auto c = GetCeiling(floor, snow->x, snow->y, snow->z);
		if (snow->y < c)
			return true;
		return false;
	}
	return true;
}
static bool CreateNewSnowflake(ITEM_INFO* item)
{
	int snowID = GetFreeSnowflake();
	if (snowID == -1)
		return false;
	auto* snow = &snowflakes_2[snowID];
	auto rad = GetRandomDraw() & 0xFFF;
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

		if (snow->stopped)
		{
			snow->life -= 12;
			if (snow->life <= 0)
				snow->life = 0;
		}

		if ((snow->yv & 7) < 7)
			snow->yv++;
	}
}

void DrawMiniSnowEffect(ITEM_INFO* item)
{
	PHDTEXTURESTRUCT tex = {};
	PHD_VECTOR pos = {};
	PHD_VBUF v[3] = {};
	bool oldBlueEffect = bBlueEffect;
	bBlueEffect = false;
#if (DIRECT3D_VERSION >= 0x900)
	auto* dm = &App.lpDeviceInfo->D3DInfo[App.lpDXConfig->nD3D].DisplayMode[App.lpDXConfig->nVMode];
#else
	auto* dm = &App.lpDeviceInfo->DDInfo[App.lpDXConfig->nDD].D3DInfo[App.lpDXConfig->nD3D].DisplayMode[App.lpDXConfig->nVMode];
#endif
	auto* sprite = &phdspriteinfo[objects[DEFAULT_SPRITES].mesh_index + 17];
	auto u1 = (sprite->offset << 8) & 0xFF00;
	auto v1 = sprite->offset & 0xFF00;
	auto u2 = ushort(u1 + sprite->width - App.nUVAdd);
	auto v2 = ushort(v1 + sprite->height - App.nUVAdd);
	u1 += (ushort)App.nUVAdd;
	v1 += (ushort)App.nUVAdd;
	v[0].u = u2;
	v[0].v = v1;
	v[1].u = u2;
	v[1].v = v2;
	v[2].u = u1;
	v[2].v = v2;

	phd_PushMatrix();
	phd_TranslateAbs(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos);

	for (int i = 0; i < MAX_MINI_WEATHER; i++)
	{
		auto* snow = &snowflakes_2[i];
		if (!snow->on)
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
		long size = phd_persp * (snow->yv >> 3) / (pos.z >> 16);
		if (size < 4)
			size = 4;
		else if (size > 16)
			size = 16;
		size = (size * 0x2AAB) >> 15; // this scales it down to about a third of the size
		size = GetFixedScale(size);

		long c;
		if ((snow->yv & 7) < 7)
			c = snow->yv & 7;
		else if (snow->life > 18)
			c = 15;
		else
			c = snow->life;
		c <<= 3;
		c = RGB_MAKE(c, c, c);
		setXYZ3(v, pos.x + size, pos.y - (size << 1), pos.z, c, pos.x + size, pos.y + size, pos.z, c, pos.x - (size << 1), pos.y + size, pos.z, c, true);
		tex.drawtype = 2;
		tex.tpage = sprite->tpage;
		HWI_InsertGT3_Poly(&v[0], &v[1], &v[2], &tex, &v[0].u, &v[1].u, &v[2].u, MID_SORT, 0);
	}

	phd_PopMatrix();

	bBlueEffect = oldBlueEffect;
}