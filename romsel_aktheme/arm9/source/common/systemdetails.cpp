#include "systemdetails.h"
#include "common/flashcard.h"

// Make this link C-like for volatility
extern "C" {
    static volatile int _batteryLevel = 0;
    static volatile int _volumeLevel = -1;
    static volatile int _sdStatus = 0;

    void batteryCallback(u32 newBatteryLevel, void *userdata) {
        _batteryLevel = newBatteryLevel;
    }


    void volumeCallback(u32 newVolumeLevel, void *userdata) {
        _volumeLevel = newVolumeLevel;
    }


    void sdStatusCallback(u32 newSdStatus, void *userdata) {
        _sdStatus = newSdStatus;
    }

    void registerFifoHandlers() {
        fifoSetValue32Handler(FIFO_USER_06, volumeCallback, NULL);
        fifoSetValue32Handler(FIFO_USER_05, batteryCallback, NULL);
        fifoSetValue32Handler(FIFO_USER_08, sdStatusCallback, NULL);
    }
}

SystemDetails::SystemDetails()
{

    _flashcardUsed = false;
    _arm7SCFGLocked = false;
    _isRegularDS = true;
    _nitroFsInitOk = false;
    _fatInitOk = false;
    _fifoOk = false;

    fifoWaitValue32(FIFO_USER_03);
	fifoWaitValue32(FIFO_USER_07);

    if (fifoGetValue32(FIFO_USER_03) == 0)
        _arm7SCFGLocked = true; // If DSiMenu++ is being run from DSiWarehax or flashcard, then arm7 SCFG is locked.
    
    u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
    if (arm7_SNDEXCNT != 0)
    {
        _isRegularDS = false; // If sound frequency setting is found, then the console is not a DS Phat/Lite
    }
    
    // force is regular ds
    //_isRegularDS = true;
    // Restore value.
}

void SystemDetails::initArm7RegStatuses() {
    if (!_fifoOk) {
        registerFifoHandlers();
    }
    _fifoOk = true;
}

int SystemDetails::batteryStatus() {
    return _batteryLevel;
}

int SystemDetails::volumeStatus() {
    return _volumeLevel;
}

SystemDetails::ESDStatus SystemDetails::sdStatus() {
    return (SystemDetails::ESDStatus)_sdStatus;
}

void SystemDetails::initFilesystem(const char *runningPath)
{
    if (_fatInitOk)
        return;

    _fatInitOk = fatInitDefault();
    int ntr = nitroFSInit("/_nds/TWiLightMenu/akmenu.srldr");
    _nitroFsInitOk = (ntr == 1);

    if (!_nitroFsInitOk && runningPath != NULL)
    {
        _nitroFsInitOk = nitroFSInit(runningPath) == 1;
    }
    flashcardInit();
}
