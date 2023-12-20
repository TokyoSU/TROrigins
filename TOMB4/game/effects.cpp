#include "../tomb4/pch.h"
#include "effects.h"
#include "box.h"
#include "sound.h"
#include "deltapak.h"
#include "objects.h"
#include "items.h"
#include "lot.h"
#include "hair.h"
#include "scarab.h"
#include "tomb4fx.h"
#include "footprnt.h"
#include "effect2.h"
#include "control.h"
#include "draw.h"
#include "lara_states.h"
#include "camera.h"
#include "lara.h"
#include "savegame.h"
#include "pickup.h"
#include "gameflow.h"
#include "../specific/function_stubs.h"
#include "../specific/function_table.h"
#include "../specific/3dmath.h"
#include "../specific/dxsound.h"
#include "../specific/polyinsert.h"
#include "sphere.h"
#include "laraswim.h"

FX_INFO* effects;
OBJECT_VECTOR* sound_effects;
long GlobalFogOff = 0;
long number_sound_effects;

DWORD FogTableColor[28] =
{
	RGBONLY(0,0,0),
	RGBONLY(245,200,60),
	RGBONLY(120,196,112),
	RGBONLY(202,204,230),
	RGBONLY(128,64,0),
	RGBONLY(64,64,64),
	RGBONLY(243,232,236),
	RGBONLY(0,64,192),
	RGBONLY(0,128,0),
	RGBONLY(150,172,157),
	RGBONLY(128,128,128),
	RGBONLY(204,163,123),
	RGBONLY(177,162,140),
	RGBONLY(0,223,191),
	RGBONLY(111,255,223),
	RGBONLY(244,216,152),
	RGBONLY(248,192,60),
	RGBONLY(252,0,0),
	RGBONLY(198,95,87),
	RGBONLY(226,151,118),
	RGBONLY(248,235,206),
	RGBONLY(0,30,16),
	RGBONLY(250,222,167),
	RGBONLY(218,175,117),
	RGBONLY(225,191,78),
	RGBONLY(77,140,141),
	RGBONLY(4,181,154),
	RGBONLY(255,174,0)
};

void(*FlipEffectRoutines[47])(ITEM_INFO* item) =
{
	EFF_Turn180, // 0
	EFF_RumbleFX,
	EFF_FloodSFX,
	EFF_DoLaraBubble,
	EFF_FinishLevel,
	EFF_ActivateCamera,
	EFF_ActivateKey,
	EFF_RubbleFXLoop,
	EFF_SwapCrowbar,
	NULL,
	EFF_PlaySoundEffect, // 10
	EFF_PlayExplosionSoundEffect,
	EFF_FreeLaraHands,
	NULL,
	EFF_LaraDrawRightGun,
	EFF_LaraDrawLeftGun,
	EFF_LaraShootRightGun,
	EFF_LaraShootLeftGun,
	EFF_SwapMeshWithMeshswap1,
	EFF_SwapMeshWithMeshswap2,
	EFF_SwapMeshWithMeshswap3, // 20
	EFF_InvisibilityOn,
	EFF_InvisibilityOff,
	NULL,
	NULL,
	NULL,
	EFF_ResetHair,
	NULL,
	EFF_SetFogColor,
	EFF_GhostTrap,
	EFF_SetLaraLocation, // 30
	EFF_ClearScarabBeetles,
	EFF_AddFootprintLeft,
	EFF_AddFootprintRight,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, // 40
	NULL,
	NULL,
	EFF_MeshSwapToWaterSkin,
	EFF_MeshSwapFromWaterSkin,
	EFF_SetLaraLocationPad,
	EFF_KillActiveCreatures // 46
};

void(*CreatureEffectRoutines[47])(ITEM_INFO* item, short extra) =
{
	EFF_ShootProjectileFromItem, // 0, Spawn or do bullet shoot.
	EFF_DoDamageFromItem, // Do hand to hand (or shockwave effect) damage.
	EFF_FootSplash, // Do foot splash effect for entity.
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, // 10
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, // 20
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, // 30
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL, // 40
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL // 46
};

void EFF_SetFogColor(ITEM_INFO* item)
{
	GlobalFogOff = 0;

	if (IsVolumetric())
	{
		if (TriggerTimer == 100)
			GlobalFogOff = 1;
		else
		{
			gfFog.r = CLRR(FogTableColor[TriggerTimer]);
			gfFog.g = CLRG(FogTableColor[TriggerTimer]);
			gfFog.b = CLRB(FogTableColor[TriggerTimer]);
			savegame.fog_colour.r = gfFog.r;
			savegame.fog_colour.g = gfFog.g;
			savegame.fog_colour.b = gfFog.b;
			SetFogColor(gfFog.r, gfFog.g, gfFog.b);
		}
	}

	flipeffect = -1;
}

void EFF_FinishLevel(ITEM_INFO* item)
{
	gfLevelComplete = gfCurrentLevel + 1;
}

void EFF_Turn180(ITEM_INFO* item)
{
	item->pos.y_rot += ANGLE(180);
	item->pos.x_rot = -item->pos.x_rot;
}

void EFF_RumbleFX(ITEM_INFO* item)
{
	long dx, dy, dz, dist;

	dx = item->pos.x_pos - camera.pos.x;
	dy = item->pos.y_pos - camera.pos.y;
	dz = item->pos.z_pos - camera.pos.z;

	if (abs(dx) < 0x4000 && abs(dy) < 0x4000 && abs(dz) < 0x4000)
	{
		dist = SQUARE(dx) + SQUARE(dy) + SQUARE(dz);
		camera.bounce = -66 * (0x100000 - dist / 256) / 0x100000;
	}
}

void EFF_PlaySoundEffect(ITEM_INFO* item)
{
	SoundEffect(TriggerTimer, 0, SFX_DEFAULT);
	flipeffect = -1;
}

void EFF_RubbleFXLoop(ITEM_INFO* item)
{
	ITEM_INFO* eq;

	eq = find_a_fucking_item(EARTHQUAKE);

	if (eq)
	{
		AddActiveItem(eq - items);
		eq->status = ITEM_ACTIVE;
		eq->flags |= IFL_CODEBITS;
	}
	else
		camera.bounce = -150;

	flipeffect = -1;
}

void EFF_FloodSFX(ITEM_INFO* item)
{
	SoundEffect(SFX_WATER_FLUSHES, 0, SFX_DEFAULT);
	flipeffect = -1;
}

void EFF_ActivateCamera(ITEM_INFO* item)
{
	KeyTriggerActive = 2;
}

void EFF_ActivateKey(ITEM_INFO* item)
{
	KeyTriggerActive = 1;
}

void EFF_SwapCrowbar(ITEM_INFO* item)
{
	short* tmp;

	tmp = meshes[objects[LARA].mesh_index + 2 * LM_RHAND];

	if (lara.mesh_ptrs[LM_RHAND] == tmp)
		lara.mesh_ptrs[LM_RHAND] = meshes[objects[CROWBAR_ANIM].mesh_index + (2 * LM_RHAND)];
	else
		lara.mesh_ptrs[LM_RHAND] = tmp;
}

void EFF_PlayExplosionSoundEffect(ITEM_INFO* item)
{
	SoundEffect(SFX_EXPLOSION1, 0, SFX_DEFAULT);
	camera.bounce = -75;
	flipeffect = -1;
}

void EFF_SetLaraLocation(ITEM_INFO* item)
{
	lara.location = TriggerTimer;
	if (TriggerTimer > lara.highest_location)
		lara.highest_location = TriggerTimer;
	flipeffect = -1;
}

void EFF_SetLaraLocationPad(ITEM_INFO* item)
{
	lara.locationPad = TriggerTimer;
	lara.location = TriggerTimer;
	flipeffect = -1;
}

void EFF_GhostTrap(ITEM_INFO* item)
{
	ITEM_INFO* target_item = NULL;
	for (short item_num = next_item_active; item_num != NO_ITEM; item_num = target_item->next_active)
	{
		target_item = &items[item_num];
		if (target_item->object_number == WRAITH3 && target_item->hit_points <= 0)
		{
			target_item->hit_points = item->index;
			break;
		}
	}

	flipeffect = -1;
}

void EFF_KillActiveCreatures(ITEM_INFO* item)
{
	ITEM_INFO* target_item = NULL;
	for (short item_num = next_item_active; item_num != NO_ITEM; item_num = target_item->next_active)
	{
		target_item = &items[item_num];
		if (item == target_item && objects[target_item->object_number].intelligent)
		{
			target_item->status = ITEM_INVISIBLE;
			if (item != NULL)
			{
				RemoveActiveItem(item_num);
				DisableBaddieAI(item_num);
				item->flags |= IFL_INVISIBLE;
			}
		}
	}

	flipeffect = -1;
}

void KillActiveBaddies(bool doRemoveAndDisable)
{
	ITEM_INFO* target_item = NULL;
	for (short item_num = next_item_active; item_num != NO_ITEM; item_num = target_item->next_active)
	{
		target_item = &items[item_num];
		if (objects[target_item->object_number].intelligent)
		{
			target_item->status = ITEM_INVISIBLE;
			if (doRemoveAndDisable)
			{
				RemoveActiveItem(item_num);
				DisableBaddieAI(item_num);
				target_item->flags |= IFL_INVISIBLE;
			}
		}
	}

	flipeffect = -1;
}

void EFF_FreeLaraHands(ITEM_INFO* item)
{
	lara.gun_status = LG_NO_ARMS;
}

void EFF_LaraDrawRightGun(ITEM_INFO* item)
{
	short* tmp;

	tmp = lara.mesh_ptrs[LM_RTHIGH];
	lara.mesh_ptrs[LM_RTHIGH] = meshes[objects[PISTOLS_ANIM].mesh_index + LM_RTHIGH * 2];
	meshes[objects[PISTOLS_ANIM].mesh_index + LM_RTHIGH * 2] = tmp;

	tmp = lara.mesh_ptrs[LM_RHAND];
	lara.mesh_ptrs[LM_RHAND] = meshes[objects[PISTOLS_ANIM].mesh_index + LM_RHAND * 2];
	meshes[objects[PISTOLS_ANIM].mesh_index + LM_RHAND * 2] = tmp;
}

void EFF_LaraDrawLeftGun(ITEM_INFO* item)
{
	short* tmp;

	tmp = lara.mesh_ptrs[LM_LTHIGH];
	lara.mesh_ptrs[LM_LTHIGH] = meshes[objects[PISTOLS_ANIM].mesh_index + LM_LTHIGH * 2];
	meshes[objects[PISTOLS_ANIM].mesh_index + LM_LTHIGH * 2] = tmp;

	tmp = lara.mesh_ptrs[LM_LHAND];
	lara.mesh_ptrs[LM_LHAND] = meshes[objects[PISTOLS_ANIM].mesh_index + LM_LHAND * 2];
	meshes[objects[PISTOLS_ANIM].mesh_index + LM_LHAND * 2] = tmp;
}

void EFF_LaraShootRightGun(ITEM_INFO* item)
{
	lara.right_arm.flash_gun = 3;
}

void EFF_LaraShootLeftGun(ITEM_INFO* item)
{
	lara.left_arm.flash_gun = 3;
}

void EFF_SwapMeshWithMeshswap1(ITEM_INFO* item)
{
	OBJECT_INFO* obj;
	short* tmp;

	obj = &objects[item->object_number];

	for (int i = 0; i < obj->nmeshes; i++)
	{
		tmp = meshes[obj->mesh_index + i * 2];
		meshes[obj->mesh_index + i] = meshes[objects[MESHSWAP1].mesh_index + i * 2];
		meshes[objects[MESHSWAP1].mesh_index + i * 2] = tmp;
	}
}

void EFF_SwapMeshWithMeshswap2(ITEM_INFO* item)
{
	OBJECT_INFO* obj;
	short* tmp;

	obj = &objects[item->object_number];

	for (int i = 0; i < obj->nmeshes; i++)
	{
		tmp = meshes[obj->mesh_index + i * 2];
		meshes[obj->mesh_index + i] = meshes[objects[MESHSWAP2].mesh_index + i * 2];
		meshes[objects[MESHSWAP2].mesh_index + i * 2] = tmp;
	}
}

void EFF_SwapMeshWithMeshswap3(ITEM_INFO* item)
{
	OBJECT_INFO* obj;
	short* tmp;

	obj = &objects[item->object_number];

	for (int i = 0; i < obj->nmeshes; i++)
	{
		tmp = meshes[obj->mesh_index + i * 2];
		meshes[obj->mesh_index + i] = meshes[objects[MESHSWAP3].mesh_index + i * 2];

		if (item == lara_item)
			lara.mesh_ptrs[i] = meshes[objects[MESHSWAP3].mesh_index + i * 2];

		meshes[objects[MESHSWAP3].mesh_index + i * 2] = tmp;
	}
}

void EFF_InvisibilityOn(ITEM_INFO* item)
{
	item->status = ITEM_INVISIBLE;
}

void EFF_InvisibilityOff(ITEM_INFO* item)
{
	item->status = ITEM_ACTIVE;
}

void EFF_ResetHair(ITEM_INFO* item)
{
	InitialiseHair();
}

void EFF_ClearScarabBeetles(ITEM_INFO* item)
{
	ClearScarabs();
}

void EFF_MeshSwapToWaterSkin(ITEM_INFO* item)
{
	lara.mesh_ptrs[LM_LHAND] = meshes[objects[item->item_flags[2]].mesh_index + LM_LHAND * 2];
}

void EFF_MeshSwapFromWaterSkin(ITEM_INFO* item)
{
	lara.mesh_ptrs[LM_LHAND] = meshes[objects[LARA_SKIN].mesh_index + LM_LHAND * 2];
}

void void_effect(ITEM_INFO* item)
{

}

void WaterFall(short item_number)
{
	ITEM_INFO* item;
	long dx, dy, dz;

	item = &items[item_number];
	dx = item->pos.x_pos - lara_item->pos.x_pos;
	dy = item->pos.y_pos - lara_item->pos.y_pos;
	dz = item->pos.z_pos - lara_item->pos.z_pos;

	if (dx >= -0x4000 && dx <= 0x4000 && dz >= -0x4000 && dz <= 0x4000 && dy >= -0x4000 && dy <= 0x4000)
	{
		if (!(wibble & 0xC))
		{
			dx = (136 * phd_sin(item->pos.y_rot)) >> 12;
			dz = (136 * phd_cos(item->pos.y_rot)) >> 12;
			TriggerWaterfallMist(item->pos.x_pos + dx, item->pos.y_pos, item->pos.z_pos + dz, item->pos.y_rot >> 4);
		}
		SoundEffect(SFX_WATERFALL_LOOP, &item->pos, 0);
	}
}

void WadeSplash(ITEM_INFO* item, long water, long depth)
{
	auto room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	if (!(room[room_number].flags & ROOM_UNDERWATER))
		return;

	auto* bounds = GetBestFrame(item);
	if (item->pos.y_pos + bounds[2] > water || item->pos.y_pos + bounds[3] < water)
		return;

	if (item->fallspeed > 0 && depth < 474 && !SplashCount)
	{
		splash_setup.x = item->pos.x_pos;
		splash_setup.y = water;
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerRad = 16;
		splash_setup.InnerSize = 12;
		splash_setup.InnerRadVel = 160;
		splash_setup.InnerYVel = -72 * item->fallspeed;
		splash_setup.pad1 = 24;
		splash_setup.MiddleRad = 24;
		splash_setup.MiddleSize = 224;
		splash_setup.MiddleRadVel = -36 * item->fallspeed;
		splash_setup.MiddleYVel = 32;
		splash_setup.pad2 = 32;
		splash_setup.OuterRad = 272;
		SetupSplash(&splash_setup);
		SplashCount = 16;
	}
	else if (!(wibble & 0xF) && (!(GetRandomControl() & 0xF) || item->current_anim_state != AS_STOP))
	{
		if (item->current_anim_state == AS_STOP)
			SetupRipple(item->pos.x_pos, water, item->pos.z_pos, (GetRandomControl() & 0xF) + 112, 16);
		else
			SetupRipple(item->pos.x_pos, water, item->pos.z_pos, (GetRandomControl() & 0xF) + 112, 18);
	}
}

void EFF_FootSplash(ITEM_INFO* item, short extra)
{
	if (extra < 0) // Don't allow negative bone !
		return;
	PHD_VECTOR pos;
	GetJointAbsPosition(item, &pos, extra);
	auto room_number = item->room_number;
	GetFloor(pos.x, pos.y, pos.z, &room_number);
	bool isFootUnderwater = room[room_number].flags & ROOM_UNDERWATER;
	if (!isFootUnderwater)
		return;
	long wh = GetWaterHeight(pos.x, pos.y, pos.z, room_number);
	auto* bounds = GetBestFrame(item);
	if ((item->pos.y_pos + bounds[2]) > wh || (item->pos.y_pos + bounds[3]) < wh)
		return;
	long fallspeed = 70;
	long wd = GetWaterDepth(pos.x, pos.y, pos.z, room_number);
	if (wd <= CLICK(1))
	{
		splash_setup.x = pos.x;
		splash_setup.y = wh + 10;
		splash_setup.z = pos.z;
		splash_setup.InnerRad = 16;
		splash_setup.InnerSize = 12;
		splash_setup.InnerRadVel = 160;
		splash_setup.InnerYVel = -72 * fallspeed;
		splash_setup.pad1 = 24;
		splash_setup.MiddleRad = 24;
		splash_setup.MiddleSize = 224;
		splash_setup.MiddleRadVel = -36 * fallspeed;
		splash_setup.MiddleYVel = 32;
		splash_setup.pad2 = 32;
		splash_setup.OuterRad = 272;
		SetupSplash(&splash_setup);
	}
	else if (!(wibble & 0xF))
	{
		splash_setup.x = pos.x;
		splash_setup.y = wh + 10;
		splash_setup.z = pos.z;
		splash_setup.InnerRad = 16;
		splash_setup.InnerSize = 12;
		splash_setup.InnerRadVel = 160;
		splash_setup.InnerYVel = -40 * fallspeed;
		splash_setup.pad1 = 24;
		splash_setup.MiddleRad = 24;
		splash_setup.MiddleSize = 224;
		splash_setup.MiddleRadVel = -20 * fallspeed;
		splash_setup.MiddleYVel = 32;
		splash_setup.pad2 = 32;
		splash_setup.OuterRad = 272;
		SetupSplash(&splash_setup);
	}
}

void Splash(ITEM_INFO* item)
{
	auto room_number = item->room_number;
	GetFloor(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, &room_number);
	if (room[room_number].flags & ROOM_UNDERWATER)
	{
		splash_setup.x = item->pos.x_pos;
		splash_setup.y = GetWaterHeight(item->pos.x_pos, item->pos.y_pos, item->pos.z_pos, room_number);
		splash_setup.z = item->pos.z_pos;
		splash_setup.InnerRad = 32;
		splash_setup.InnerSize = 8;
		splash_setup.InnerRadVel = 320;
		splash_setup.InnerYVel = -40 * item->fallspeed;
		splash_setup.pad1 = 48;
		splash_setup.MiddleRad = 32;
		splash_setup.MiddleSize = 480;
		splash_setup.MiddleRadVel = -20 * item->fallspeed;
		splash_setup.MiddleYVel = 32;
		splash_setup.pad2 = 128;
		splash_setup.OuterRad = 544;
		SetupSplash(&splash_setup);
	}
}

short DoBloodSplat(long x, long y, long z, short speed, short ang, short room_number)
{
	if (room[room_number].flags & ROOM_UNDERWATER)
		TriggerUnderwaterBlood(x, y, z, speed);
	else
		TriggerBlood(x, y, z, ang >> 4, speed);
	return -1;
}

void DoLotsOfBlood(long x, long y, long z, short speed, short ang, short room_number, long num)
{
	for (; num > 0; num--)
	{
		auto bx = x - (GetRandomControl() << 9) / 0x8000 + 256;
		auto by = y - (GetRandomControl() << 9) / 0x8000 + 256;
		auto bz = z - (GetRandomControl() << 9) / 0x8000 + 256;
		DoBloodSplat(bx, by, bz, speed, ang, room_number);
	}
}

void Richochet(GAME_VECTOR* pos)
{
	TriggerRicochetSpark(pos, mGetAngle(pos->z, pos->x, lara_item->pos.z_pos, lara_item->pos.x_pos) >> 4, 3, 0);
	SoundEffect(SFX_LARA_RICOCHET, (PHD_3DPOS*)pos, SFX_DEFAULT);
}

void SoundEffects()
{
	OBJECT_VECTOR* sfx;
	SoundSlot* slot;

	for (int i = 0; i < number_sound_effects; i++)
	{
		sfx = &sound_effects[i];

		if (flip_status)
		{
			if (sfx->flags & 0x40)
				SoundEffect(sfx->data, (PHD_3DPOS*)sfx, 0);
		}
		else if (sfx->flags & 0x80)
			SoundEffect(sfx->data, (PHD_3DPOS*)sfx, 0);
	}

	if (flipeffect != -1 && FlipEffectRoutines[flipeffect] != NULL)
		FlipEffectRoutines[flipeffect](NULL);

	if (!sound_active)
		return;

	for (int i = 0; i < 32; i++)
	{
		slot = &LaSlot[i];
		if (slot->nSampleInfo < 0)
			continue;

		if ((sample_infos[slot->nSampleInfo].flags & 3) != 3)
		{
			if (!S_SoundSampleIsPlaying(i))
				slot->nSampleInfo = -1;
			else
			{
				GetPanVolume(slot);
				S_SoundSetPanAndVolume(i, (short)slot->nPan, (ushort)slot->nVolume);
			}
		}
		else
		{
			if (!slot->nVolume)
			{
				S_SoundStopSample(i);
				slot->nSampleInfo = -1;
			}
			else
			{
				S_SoundSetPanAndVolume(i, (short)slot->nPan, (ushort)slot->nVolume);
				S_SoundSetPitch(i, slot->nPitch);
				slot->nVolume = 0;
			}
		}
	}
}

long ItemNearLara(PHD_3DPOS* pos, long rad)
{
	auto dx = pos->x_pos - lara_item->pos.x_pos;
	auto dy = pos->y_pos - lara_item->pos.y_pos;
	auto dz = pos->z_pos - lara_item->pos.z_pos;
	if (dx >= -rad && dx <= rad && dz >= -rad && dz <= rad && dy >= -SECTOR(3) && dy <= SECTOR(3) && SQUARE(dx) + SQUARE(dz) <= SQUARE(rad))
	{
		auto* bounds = GetBoundsAccurate(lara_item);
		if (dy >= bounds[2] && dy <= bounds[3] + 100)
			return TRUE;
	}
	return FALSE;
}
