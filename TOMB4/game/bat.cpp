#include "../tomb4/pch.h"
#include "bat.h"
#include "box.h"
#include "objects.h"
#include "../specific/function_stubs.h"
#include "effects.h"
#include "lara.h"
#include "control.h"
#include "lot.h"

enum BAT_STATE
{
	BAT_STATE_NONE,
	BAT_STATE_START_FLY,
	BAT_STATE_FLY,
	BAT_STATE_ATTACK,
	BAT_STATE_DEATH_FALL,
	BAT_STATE_DEATH,
	BAT_STATE_WAIT
};

static BITE_INFO bat_bite = { 0, 16, 45, 4 };

#define BAT_DAMAGE 2
#define BAT_FLY_ANIM 1
#define BAT_DEATH_START_ANIM 3
#define BAT_WAIT_ANIM 5

void InitialiseBat(short item_number)
{
	auto* item = &items[item_number];
	InitialiseCreature(item_number);
	SetAnimation(item, BAT_WAIT_ANIM, BAT_STATE_WAIT);
}

void BatControl(short item_number)
{
	if (!CreatureActive(item_number))
		return;

	auto* item = &items[item_number];
	auto* bat = GetCreatureInfo(item);
	short angle = 0;

	if (item->hit_points <= 0)
	{
		switch (item->current_anim_state)
		{
		case BAT_STATE_ATTACK:
			SetAnimation(item, BAT_FLY_ANIM, BAT_STATE_FLY, 0);
			break;
		default:
			if (item->pos.y_pos >= item->floor)
			{
				item->goal_anim_state = BAT_STATE_DEATH;
				item->pos.y_pos = item->floor;
				item->gravity_status = FALSE;
			}
			else
			{
				item->speed = 0;
				item->fallspeed = 0;
				item->gravity_status = TRUE;
				SetAnimation(item, BAT_DEATH_START_ANIM, BAT_STATE_DEATH_FALL, 0);
			}
			break;
		}
	}
	else
	{
		if (item->ai_bits != 0)
			GetAITarget(bat);
		else
			bat->enemy = GetNearestTarget(item, VON_CROY);

		AI_INFO info;
		CreatureAIInfo(item, &info);
		GetCreatureMood(item, &info, FALSE);
		if (bat->flags != 0)
			bat->mood = ESCAPE_MOOD;
		CreatureMood(item, &info, FALSE);
		angle = CreatureTurn(item, ANGLE(20));

		switch (item->current_anim_state)
		{
		case BAT_STATE_WAIT:
			if (info.distance < 0x1900000 || item->hit_status || bat->hurt_by_lara)
				item->goal_anim_state = BAT_STATE_START_FLY;

			break;

		case BAT_STATE_FLY:
			if (info.distance < 0x10000 || !(GetRandomControl() & 0x3F))
				bat->flags = 0;
			if (bat->flags == 0 && (item->touch_bits != 0 || (bat->enemy != lara_item && info.distance < 0x10000 && info.ahead && abs(item->pos.y_pos - bat->enemy->pos.y_pos) < 896)))
				item->goal_anim_state = BAT_STATE_ATTACK;

			break;

		case BAT_STATE_ATTACK:
			if (bat->flags == 0 && (item->touch_bits != 0 || (bat->enemy != lara_item && info.distance < 0x10000 && info.ahead && abs(item->pos.y_pos - bat->enemy->pos.y_pos) < 896)))
			{
				CreatureEffect(item, &bat_bite, DoBloodSplat);
				if (bat->enemy == lara_item)
					DamageTarget(item, bat->enemy, BAT_DAMAGE);
				bat->flags = 1;
			}
			else
			{
				item->goal_anim_state = BAT_STATE_FLY;
				bat->mood = BORED_MOOD;
			}

			break;
		}
	}

	CreatureAnimation(item_number, angle, 0);
}
