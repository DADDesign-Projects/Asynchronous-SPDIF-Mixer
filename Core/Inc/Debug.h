
#include "stdint.h"
#pragma once
struct sDebug{
	int32_t CtErrPlus;

	int32_t ctDate;
	int32_t ctDateMax;
	int32_t ctDateMin;

	int32_t DeltaIn1;
	int32_t DeltaIn1Min;
	int32_t DeltaIn1Max;

};
#define MinMax(Cible, Min, Max) Min = Cible < Min ? Cible : Min; Max = Cible > Max ? Cible : Max
extern sDebug __Debug;
