#include "../tomb4/pch.h"
#include "lasers.h"
#include "../game/control.h"
#include "../game/draw.h"
#include "../game/lara.h"
#include "../game/objects.h"
#include "../specific/specificfx.h"
#include "effects.h"
#include "../specific/function_stubs.h"

void InitialiseLaser(short item_num)
{
}

static bool LaraOnLOS(GAME_VECTOR* s, GAME_VECTOR* t)
{
	MESH_INFO* mesh = NULL;
	PHD_VECTOR pos;

	long itemNo = ObjectOnLOS2(s, t, &pos, &mesh, true);
	if (itemNo != NO_LOS && itemNo != NO_ITEM)
	{
		auto* item = &items[itemNo];
		if (item->object_number == LARA)
			return true;
	}

	return false;
}

void LaserControl(short item_num)
{

}

void DrawLaser(ITEM_INFO* item)
{
	if (!TriggerActive(item))
		return;

	GAME_VECTOR s, t;
	long x = 0, y = 0, z = 0;
	switch (item->pos.y_rot)
	{
	case ANGLE(0):
		x = 0;
		z = 511;
		break;
	case ANGLE(90):
		x = 511;
		z = 0;
		break;
	case -ANGLE(90):
		x = 0;
		z = -511;
		break;
	case -ANGLE(180):
		x = -511;
		z = 0;
		break;
	}

	for (long lp = 0; lp < 32; lp++)
	{
		s.x = item->pos.x_pos + x;
		s.y = item->pos.y_pos + y;
		s.z = item->pos.z_pos + z;
		s.room_number = item->room_number;
		t.x = item->pos.x_pos + (x << 5);
		t.y = item->pos.y_pos + y;
		t.z = item->pos.z_pos + (z << 5);
		LOS(&s, &t);

		if (LaraOnLOS(&s, &t))
		{
			switch (item->object_number)
			{
			case LASER_NORMAL:
				TestTriggers(trigger_index, TRUE, 0);
				break;
			case LASER_DAMAGE:
				lara_item->hit_points -= 10;
				DoLotsOfBlood(lara_item->pos.x_pos, item->pos.y_pos + y, lara_item->pos.z_pos, (GetRandomDraw() & 0x7F) + 128, short(GetRandomDraw() << 1), lara_item->room_number, 1);
				break;
			case LASER_KILL:
				lara_item->hit_points = 0;
				DoLotsOfBlood(lara_item->pos.x_pos, item->pos.y_pos + y, lara_item->pos.z_pos, (GetRandomDraw() & 0x7F) + 128, short(GetRandomDraw() << 1), lara_item->room_number, 1);
				break;
			}
		}

		switch (item->object_number)
		{
		case LASER_NORMAL:
			S_DrawLaser(item, &s, &t, 255, 255, 255);
			break;
		case LASER_DAMAGE:
			S_DrawLaser(item, &s, &t, 255, 255, 255);
			break;
		case LASER_KILL:
			S_DrawLaser(item, &s, &t, 255, 255, 255);
			break;
		}
	}
}
