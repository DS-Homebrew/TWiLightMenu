#include <nds/arm9/dldi.h>
#include "systemdetails.h"
#include "common/flashcard.h"
#include "common/arm7status.h"

#define CHECK_BIT(v, n) (((v) >> (n)) & 1)

// Make this link C-like for volatility
extern "C" {
    static volatile int _batteryLevel = 0;
    static volatile int _volumeLevel = -1;
    static volatile int _sdStatus = 0;

    void volBatSdCallback(u32 status, void *userdata) {
        _batteryLevel = (status & BAT_MASK) >> BAT_OFF;
        _volumeLevel = (status & VOL_MASK) >> VOL_OFF;
        _sdStatus = (status & SD_MASK) >> SD_OFF;
    }

    void registerFifoHandlers() {
        fifoSetValue32Handler(FIFO_USER_03, volBatSdCallback, NULL);
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

    // status (Bit 0: isDSLite, Bit 1: scfgEnabled, Bit 2: REG_SNDEXTCNT)
    u32 status = ((fifoGetValue32(FIFO_USER_03)) >> INIT_OFF);

    if (CHECK_BIT(status, REGSCFG_BIT) == 0) 
    {
        _arm7SCFGLocked = true; // If TWiLight Menu++ is being run from DSiWarehax or flashcard, then arm7 SCFG is locked.
    }

    if (CHECK_BIT(status, REG_SNDEXTCNT_BIT) != 0)
    {
        _isRegularDS = false; // If sound frequency setting is found, then the console is not a DS Phat/Lite
    }
    
	_isDSLite = CHECK_BIT(status, DSLITE_BIT);

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
	extern const DISC_INTERFACE __my_io_dsisd;

    if (_fatInitOk) {
        return;
	}

	*(u32*)(0x2FFFD0C) = 0x54494D52;	// Run reboot timer
	fatMountSimple("sd", &__my_io_dsisd);
	fatMountSimple("fat", dldiGetInternal());
    _fatInitOk = (sdFound() || flashcardFound());
	*(u32*)(0x2FFFD0C) = 0;
	chdir(sdFound()&&isDSiMode() ? "sd:/" : "fat:/");
    int ntr = nitroFSInit("/_nds/TWiLightMenu/akmenu.srldr");
    _nitroFsInitOk = (ntr == 1);

    if (!_nitroFsInitOk && runningPath != NULL)
    {
        _nitroFsInitOk = nitroFSInit(runningPath) == 1;
    }
}
