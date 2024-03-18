#pragma once
#include "../global/types.h"

void ProjectPHDVBuf(FVECTOR* pos, PHD_VBUF* v, ulong c, bool cFlag);
void setXYZ3(PHD_VBUF* v,
	long x1, long y1, long z1, long xv1, long yv1, long c1,
	long x2, long y2, long z2, long xv2, long yv2, long c2,
	long x3, long y3, long z3, long xv3, long yv3, long c3, bool divideByPersp = true);
void setXYZ3(PHD_VBUF* v,
	long x1, long y1, long z1, long c1,
	long x2, long y2, long z2, long c2,
	long x3, long y3, long z3, long c3, bool divideByPersp = true);
void setXYZ4(PHD_VBUF* v,
	long x1, long y1, long z1, long xv1, long yv1, long c1,
	long x2, long y2, long z2, long xv2, long yv2, long c2,
	long x3, long y3, long z3, long xv3, long yv3, long c3,
	long x4, long y4, long z4, long xv4, long yv4, long c4, bool divideByPersp = true);
void setXYZ4(PHD_VBUF* v,
	long x1, long y1, long z1, long c1,
	long x2, long y2, long z2, long c2,
	long x3, long y3, long z3, long c3,
	long x4, long y4, long z4, long c4, bool divideByPersp = true);

void LaraElectricDeath(long lr, ITEM_INFO* item);
bool ClipLine(long& x1, long& y1, long& x2, long& y2, long w, long h);
void S_DrawWakeFX(ITEM_INFO* item);
void DoRain();
void DoSnow();
void DrawExplosionRings();
void DrawSummonRings();
void DrawKnockBackRings();
void TriggerElectricBeam(ITEM_INFO* item, GAME_VECTOR* src, long copy);
void TriggerTribeBossHeadElectricity(ITEM_INFO* item, long copy);
void DrawTonyBossShield(ITEM_INFO* item);
void DrawTribeBossShield(ITEM_INFO* item);
void DrawLondonBossShield(ITEM_INFO* item);
void DrawWillBossShield(ITEM_INFO* item);
void S_DrawLaserBeam(GAME_VECTOR* src, GAME_VECTOR* dest, uchar cr, uchar cg, uchar cb);
void S_DrawBat();
void S_DrawSparks();
void S_DrawSplashes();
void S_DrawLaserBolts(ITEM_INFO* item);
void S_DrawFish(ITEM_INFO* item);
void S_DrawDarts(ITEM_INFO* item);
void S_PrintSpriteShadow(short size, short* box, ITEM_INFO* item);
void S_DrawFootPrints();
void DoUwEffect();
void S_DrawSubWakeFX(ITEM_INFO* item);
void SuperDrawBox(short* bounds, long col);
void SuperDrawBox(long x, long y, long z, short* bounds, long col);

extern EXPLOSION_RING ExpRings[6];
extern WAKE_PTS WakePts[32][2];
extern uchar WakeShade;
extern uchar CurrentStartWake;
