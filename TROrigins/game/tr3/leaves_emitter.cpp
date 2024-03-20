#include "pch.h"
#include "leaves_emitter.h"
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

/// item->ocb = leaves spawning range in block.

constexpr auto MAX_MINI_WEATHER = 64;

static int GetFreeLeave(SNOWFLAKE* flake)
{
	for (int i = 0; i < MAX_MINI_WEATHER; i++)
	{
		auto* leave = &flake[i];
		if (!leave->on)
			return i;
	}
	return -1;
}

static inline SNOWFLAKE* GetLeaveList(ITEM_INFO* item)
{
	return static_cast<SNOWFLAKE*>(item->data);
}

static void UpdateLeaveRoom(SNOWFLAKE* leave, long height)
{
	short new_room = leave->room_number;
	GetFloor(leave->x, leave->y + height, leave->z, &new_room);
	if (new_room != leave->room_number)
		leave->room_number = new_room;
}

static bool IsLeaveBlocked(SNOWFLAKE* leave)
{
	auto current_room = leave->room_number;
	auto x = leave->x;
	auto y = leave->y;
	auto z = leave->z;
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

static void SpawnAtTheCeiling(SNOWFLAKE* leave)
{
	auto current_room = leave->room_number;
	auto* floor = GetFloor(leave->x, leave->y, leave->z, &current_room);
	auto c = GetCeiling(floor, leave->x, leave->y, leave->z);
	leave->y = c+1;
}

static bool CreateNewLeave(ITEM_INFO* item)
{
	auto* leaveList = GetLeaveList(item);
	int leaveID = GetFreeLeave(leaveList);
	if (leaveID == -1)
		return false;
	auto* leave = &leaveList[leaveID];
	auto rad = GetRandomDraw() & (item->ocb > 0 ? (item->ocb - 1) : (SECTOR(4) - 1));
	auto angle = GetRandomDraw() & 0x1FFE;
	leave->x = item->pos.x_pos + ((rad * rcossin_tbl[angle]) >> 12);
	SpawnAtTheCeiling(leave);
	leave->z = item->pos.z_pos + ((rad * rcossin_tbl[angle + 1]) >> 12);
	leave->room_number = item->room_number;
	if (IsLeaveBlocked(leave)) // NOTE: it will update the room_number !
		return false;
	leave->stopped = FALSE;
	leave->on = TRUE;
	leave->size = (GetRandomDraw() & 12); // NOTE: will clamp to 4 when drawing if the value is less than 4
	leave->xv = (GetRandomDraw() & 7) - 4;
	leave->yv = (GetRandomDraw() % 16 + 8) << 3;
	leave->zv = (GetRandomDraw() & 7) - 4;
	leave->life = 1024 - (leave->yv << 1);
	leave->color = UCHAR_MAX;
	return true;
}

void InitialiseLeavesEmitter(short item_number)
{
	auto* item = &items[item_number];
	item->data = (SNOWFLAKE*)game_malloc(sizeof(SNOWFLAKE) * MAX_MINI_WEATHER);
	auto* leaveList = GetLeaveList(item);
	for (int i = 0; i < MAX_MINI_WEATHER; i++) // WARM it !
		ControlLeavesEmitter(item_number);
}

void ControlLeavesEmitter(short item_number)
{
	auto* item = &items[item_number];
	auto* leaveList = GetLeaveList(item);
	for (int i = 0; i < MAX_MINI_WEATHER; i++)
	{
		CreateNewLeave(item);
		auto* leave = &leaveList[i];
		auto ox = leave->x;
		auto oy = leave->y;
		auto oz = leave->z;

		if (!leave->stopped)
		{
			leave->x += leave->xv;
			leave->y += (leave->yv & 0xFF) >> 3;
			leave->z += leave->zv;
			UpdateLeaveRoom(leave, 16);
			if (IsLeaveBlocked(leave))
			{
				leave->stopped = TRUE;
				leave->x = ox;
				leave->y = oy;
				leave->z = oz;
				if (leave->yv > 16)
					leave->yv = 0;
			}
		}

		if (leave->life <= 0)
		{
			leave->on = FALSE;
			continue;
		}

		// Does room is outside and not underwater ?
		if ((room[leave->room_number].flags & ROOM_NOT_INSIDE) && !(room[leave->room_number].flags & ROOM_UNDERWATER))
		{
			if (leave->xv < SmokeWindX << 1)
				leave->xv++;
			else if (leave->xv > SmokeWindX << 1)
				leave->xv--;
			if (leave->zv < SmokeWindZ << 1)
				leave->zv++;
			else if (leave->zv > SmokeWindZ << 1)
				leave->zv--;
		}

		leave->life--;
		if (leave->life <= 0)
			leave->life = 0;
		if (leave->stopped)
		{
			leave->life = 1; // Don't let the leave die until color is full black (or transparent if alphablend enabled)
			leave->color -= 3;
			if (leave->color <= 0)
				leave->color = 0;
			if (leave->color == 0)
				leave->on = FALSE;
		}

		if ((leave->yv & 7) != 7)
			leave->yv++;
	}
}

void DrawLeavesEmitter(ITEM_INFO* item)
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
	auto* snowflakeList = GetLeaveList(item);

	// Get the leave mesh (sprite)
	auto* sprite = &phdspriteinfo[objects[DEFAULT_SPRITES].mesh_index + ST_LEAVE];
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
		auto* snow = &snowflakeList[i];
		if (!snow->on)
			continue;
		// If the snow is no more in the view distance of the camera, just don't draw it.
		if (abs(CamPos.x - snow->x) > phd_viewdist || abs(CamPos.z - snow->z) > phd_viewdist)
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
		size = GetFixedScale(size) << 1;

		auto c = RGB_MAKE(snow->color, snow->color, snow->color);
		setXYZ4(v, // this drawing is based on the center of the texture.
			pos.x - size, pos.y - size, pos.z, c, // up-left
			pos.x + size, pos.y - size, pos.z, c, // up-right
			pos.x + size, pos.y + size, pos.z, c, // down-right
			pos.x - size, pos.y + size, pos.z, c, // down-left
			true
		);
		ZeroMemory(&tex, sizeof(tex));
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
		HWI_InsertGT4_Sorted(&v[0], &v[1], &v[2], &v[3], &tex, MID_SORT, 0);
	}
	phd_PopMatrix();

	bBlueEffect = oldBlueEffect;
}