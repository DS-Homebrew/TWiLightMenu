#include <nds/arm9/dldi.h>
#include "systemdetails.h"
#include "common/flashcard.h"
#include <slim.h>
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
    _isDSLite = false;
    _nitroFsInitOk = false;
    _fatInitOk = false;
    _fifoOk = false;

    fifoWaitValue32(FIFO_USER_03);
	fifoWaitValue32(FIFO_USER_07);

    if (fifoGetValue32(FIFO_USER_03) == 0)
        _arm7SCFGLocked = true; // If TWiLight Menu++ is being run from DSiWarehax or flashcard, then arm7 SCFG is locked.
    
    u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
    if (arm7_SNDEXCNT != 0)
    {
        _isRegularDS = false; // If sound frequency setting is found, then the console is not a DS Phat/Lite
    }
    
	_isDSLite = fifoGetValue32(FIFO_USER_04);

    // force is regular ds
    //_isRegularDS = true;
    // Restore value.
}

void SystemDetails::initArm7RegStatuses() {
	//printf("sys().initArm7RegStatuses()\n");
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
	extern const DISC_INTERFACE __my_io_dsisd;

	//printf("sys().initFilesystem()\n");
    if (_fatInitOk) {
        return;
	}

	*(u32*)(0x2FFFD0C) = 0x54494D52;	// Run reboot timer

	fatMountSimple("sd:/", &__my_io_dsisd);
    fatMountSimple("fat:/", dldiGetInternal());

    // fatInitDefault();

    _fatInitOk = (sdFound() || flashcardFound());
	*(u32*)(0x2FFFD0C) = 0;
	chdir(sdFound() && isDSiMode() ? "sd:/" : "fat:/");
    int ntr = nitroFSInit("/_nds/TWiLightMenu/dsimenu.srldr");
    _nitroFsInitOk = (ntr == 1);

    if (!_nitroFsInitOk && runningPath != NULL)
    {
        _nitroFsInitOk = nitroFSInit(runningPath) == 1;
    }
}
