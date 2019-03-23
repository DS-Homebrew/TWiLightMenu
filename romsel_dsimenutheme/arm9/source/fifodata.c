#include "fifodata.h"


static volatile int batteryLevel = 0;
static volatile int volumeLevel = 0;
static volatile int sdLevel = 0;

static volatile void batteryCallback(u32 newBatteryLevel, void *userdata) {
    batteryLevel = newBatteryLevel;
}


static volatile void volumeCallback(u32 newVolumeLevel, void *userdata) {
    volumeLevel = newVolumeLevel;
}


void registerFifoHandlers() {
    fifoSetValue32Handler(FIFO_USER_06, volumeCallback, NULL);
    fifoSetValue32Handler(FIFO_USER_05, batteryCallback, NULL);
}

int getBatteryLevelFromArm7() {
    return batteryLevel;
}


int getVolumeLevelFromArm7() {
    return volumeLevel;
}

int getSdStatusLevelFromArm7() {
    return 0;
}