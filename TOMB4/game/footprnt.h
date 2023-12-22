#pragma once
#include "../global/types.h"

int GetFreeFootprints();
void EFF_AddFootprintLeft(ITEM_INFO* item);
void EFF_AddFootprintRight(ITEM_INFO* item);
void ClearFootPrints();
void S_DrawFootPrints();

extern FOOTPRINT FootPrint[MAX_FOOTPRINTS];