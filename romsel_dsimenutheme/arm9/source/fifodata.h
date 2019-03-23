#include <nds.h>

#ifndef __FIFODATA__
#define __FIFODATA__


#ifdef __cplusplus
extern "C" {
#endif

void registerFifoHandlers();
int getBatteryLevelFromArm7();
int getVolumeLevelFromArm7();
int getSdStatusLevelFromArm7();

#ifdef __cplusplus
}
#endif
#endif