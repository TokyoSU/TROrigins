#pragma once
#include "../global/types.h"

void EFF_AddFootprintLeft(ITEM_INFO* item);
void EFF_AddFootprintRight(ITEM_INFO* item);
void S_DrawFootPrints();

extern FOOTPRINT FootPrint[];
extern long FootPrintNum;