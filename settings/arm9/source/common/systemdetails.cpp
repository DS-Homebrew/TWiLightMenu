#include "systemdetails.h"
SystemDetails::SystemDetails()
{

    _flashcardUsed = false;
    _arm7SCFGLocked = false;
    _isRegularDS = true;
    _nitroFsInitOk = false;
    _fatInitOk = false;


	fifoWaitValue32(FIFO_USER_06);
    if (fifoGetValue32(FIFO_USER_03) == 0)
        _arm7SCFGLocked = true; // If DSiMenu++ is being run from DSiWarehax or flashcard, then arm7 SCFG is locked.
    
    u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
    if (arm7_SNDEXCNT != 0)
    {
        _isRegularDS = false; // If sound frequency setting is found, then the console is not a DS Phat/Lite
    }
    
    // Restore value.
    fifoSendValue32(FIFO_USER_07, arm7_SNDEXCNT);
}

void SystemDetails::initFilesystem(const char *runningPath)
{
    if (_fatInitOk) {
        return;
	}

	*(u32*)(0x2FFFD0C) = 0x54494D52;	// Run reboot timer
    _fatInitOk = fatInitDefault();
	*(u32*)(0x2FFFD0C) = 0;
    int ntr = nitroFSInit("/_nds/TWiLightMenu/settings.srldr");
    _nitroFsInitOk = (ntr == 1);

    if (!_nitroFsInitOk && runningPath != NULL)
    {
        _nitroFsInitOk = nitroFSInit(runningPath) == 1;
    }
}
