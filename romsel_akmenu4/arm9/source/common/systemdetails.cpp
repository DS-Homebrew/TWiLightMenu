#include "systemdetails.h"
#include <nds.h>
#include <stdio.h>
#include "tool/dbgtool.h"
SystemDetails::SystemDetails()
{

    _flashcardUsed = false;
    _arm7SCFGLocked = false;
    _isRegularDS = true;

    fifoWaitValue32(FIFO_USER_06);
    if (fifoGetValue32(FIFO_USER_03) == 0)
        _arm7SCFGLocked = true; // If DSiMenu++ is being ran from DSiWarehax or flashcard, then arm7 SCFG is locked.
    u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
    if (arm7_SNDEXCNT != 0)
    {
        dbg_printf("Is DSi\n");
        _isRegularDS = false; // If sound frequency setting is found, then the console is not a DS Phat/Lite
    }
    fifoSendValue32(FIFO_USER_07, 0);

    if (!access("fat:/", F_OK))
    {
        _flashcardUsed = true;
        dbg_printf("flahcart used..\n");
    }
}

void SystemDetails::initFilesystem(const char *nitrofsPath, const char *runningPath)
{
    if (_fatInitOk)
        return;

    _fatInitOk = fatInitDefault();
    _nitroFsInitOk = (bool)nitroFSInit(nitrofsPath);

    if (!_nitroFsInitOk && runningPath != NULL)
    {
        _nitroFsInitOk = (bool)nitroFSInit(runningPath);
    }
    else
    {
        _nitroFsInitOk = false;
    }
}