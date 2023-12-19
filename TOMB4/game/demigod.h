#pragma once
#include "../global/types.h"

void TriggerDemigodMissile(PHD_3DPOS* pos, short room_number, short type);
void TriggerDemigodMissileFlame(short fx_number, long xv, long yv, long zv);
void TriggerHammerSmoke(long x, long y, long z, long num);
void DoDemigodProjectiles(ITEM_INFO* item);
void DoDemigodHammerSlam(ITEM_INFO* item);
void Demigod1Control(short item_number);
void Demigod2Control(short item_number);
void Demigod3Control(short item_number);
