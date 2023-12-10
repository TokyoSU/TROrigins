#include "../tomb4/pch.h"
#include "sentrygun.h"
#include "effect2.h"
#include "../specific/function_stubs.h"
#include "sphere.h"
#include "box.h"
#include "tomb4fx.h"
#include "lot.h"
#include "items.h"
#include "deltapak.h"
#include "objects.h"
#include "sound.h"
#include "people.h"
#include "newinv.h"
#include "control.h"

constexpr auto MESH_GUN_RAIL = 4;
constexpr auto MESH_GUN_FLASH = 8;
constexpr auto MESH_JERRICAN = 6;
static BITE_INFO AGOffsets = BITE_INFO(0, 0, 0, MESH_GUN_RAIL);

void TriggerAutogunFlamethrower(ITEM_INFO* item)
{
	SPARKS* sptr;
	PHD_VECTOR pos;
	PHD_VECTOR vel;
	long v;

	for (int i = 0; i < 3; i++)
	{
		sptr = &spark[GetFreeSpark()];
		sptr->On = 1;
		sptr->sR = (GetRandomControl() & 0x1F) + 48;
		sptr->sG = 48;
		sptr->sB = 255;
		sptr->dR = (GetRandomControl() & 0x3F) + 192;
		sptr->dG = (GetRandomControl() & 0x3F) + 128;
		sptr->dB = 32;
		sptr->ColFadeSpeed = 12;
		sptr->FadeToBlack = 8;
		sptr->TransType = 2;
		sptr->Life = (GetRandomControl() & 0x1F) + 16;
		sptr->sLife = sptr->Life;

		pos.x = -140;
		pos.y = -30;
		pos.z = -4;
		GetJointAbsPosition(item, &pos, 7);
		sptr->x = (GetRandomControl() & 0x1F) + pos.x - 16;
		sptr->y = (GetRandomControl() & 0x1F) + pos.y - 16;
		sptr->z = (GetRandomControl() & 0x1F) + pos.z - 16;

		vel.x = -280;
		vel.y = -30;
		vel.z = -4;
		GetJointAbsPosition(item, &vel, 7);
		v = (GetRandomControl() & 0x3F) + 192;
		sptr->Xvel = short(v * (vel.x - pos.x) / 10);
		sptr->Yvel = short(v * (vel.y - pos.y) / 10);
		sptr->Zvel = short(v * (vel.z - pos.z) / 10);

		sptr->Friction = 85;
		sptr->MaxYvel = 0;
		sptr->Gravity = -16 - (GetRandomControl() & 0x1F);
		sptr->flags = 538;

		if (GlobalCounter & 1)
		{
			v = 255;
			sptr->flags = 539;
		}

		sptr->Scalar = 3;
		v *= (GetRandomControl() & 7) + 60;
		sptr->dSize = uchar(v >> 8);
		sptr->Size = uchar(v >> 12);
		sptr->sSize = sptr->Size;
	}
}

void InitialiseAutogun(short item_number)
{
	auto* item = &items[item_number];
	InitialiseCreature(item_number);
	item->HideMesh(MESH_GUN_FLASH);
	item->item_flags[0] = 0;
	item->item_flags[1] = 768;
	item->item_flags[2] = 0;
}

void AutogunControl(short item_number)
{
	if (!CreatureActive(item_number))
		return;
	auto* item = &items[item_number];
	auto* autogun = GetCreatureInfo(item);
	if (autogun == NULL)
		return;
	AI_INFO info;
	PHD_VECTOR pos;
	short angle = 0;
	short ahead = 0;

	if (item->IsMeshHidden(MESH_JERRICAN))
	{
		CreatureDie(item_number, TRUE, EDF_FIRE|EDF_EXPLODE);
		TriggerExplosion(item, 768);
		SoundEffect(SFX_EXPLOSION1, &item->pos, 0x1800000 | SFX_SETPITCH);
		SoundEffect(SFX_EXPLOSION2, &item->pos, SFX_DEFAULT);
	}
	else
	{
		if (item->item_flags[0] != 0)
		{
			pos.x = AGOffsets.x;
			pos.y = AGOffsets.y;
			pos.z = AGOffsets.z;
			GetJointAbsPosition(item, &pos, AGOffsets.mesh_num);
			TriggerDynamic(pos.x, pos.y, pos.z, (item->item_flags[0] << 2) + 12, 24, 16, 4);
			item->item_flags[0]--;
		}

		if (item->item_flags[0] != 0)
			item->DrawMesh(MESH_GUN_FLASH);
		else
			item->HideMesh(MESH_GUN_FLASH);

		if (item->ocb == 0)
		{
			item->pos.y_pos -= 512;
			CreatureAIInfo(item, &info);
			item->pos.y_pos += 512;
			ahead = info.angle - autogun->joint_rotation[0];

			if (ahead > -0x4000 && ahead < 0x4000)
				info.ahead = 1;
			else
				info.ahead = 0;

			if (Targetable(item, &info) && info.distance < 0x5100000)
			{
				if (!have_i_got_object(PUZZLE_ITEM5) && !item->item_flags[0])
				{
					if (info.distance > 0x400000)
					{
						item->item_flags[0] = 2;
						ShotLara(item, &info, &AGOffsets, autogun->joint_rotation[0], 5);
						SoundEffect(SFX_AUTOGUNS, &item->pos, SFX_DEFAULT);
						item->item_flags[2] += 256;

						if (item->item_flags[2] > 6144)
							item->item_flags[2] = 6144;
					}
					else
					{
						TriggerAutogunFlamethrower(item);
						angle = (4 * rcossin_tbl[(2048 * (GlobalCounter & 0x1F)) >> 3]) >> 2;
					}
				}

				angle += info.angle - autogun->joint_rotation[0];

				if (angle > 1820)
					angle = 1820;
				else if (angle < -1820)
					angle = -1820;

				autogun->joint_rotation[0] += (short)angle;
				CreatureJoint(item, 1, -info.x_angle);
			}

			item->item_flags[2] -= 32;
			if (item->item_flags[2] < 0)
				item->item_flags[2] = 0;

			autogun->joint_rotation[2] += item->item_flags[1];
			autogun->joint_rotation[3] += item->item_flags[2];

			if (autogun->joint_rotation[2] > 0x4000 || autogun->joint_rotation[2] < -0x4000)
				item->item_flags[1] = -item->item_flags[1];
		}
		else
		{
			CreatureJoint(item, 1, 0x2000);
			CreatureJoint(item, 2, (GetRandomControl() & 0x3FFF) - 0x2000);
			CreatureJoint(item, 0, (GetRandomControl() & 0x7FF) - 1024);
		}
	}
}
