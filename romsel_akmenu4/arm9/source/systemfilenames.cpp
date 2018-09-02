#include "systemfilenames.h"
#include "common/systemdetails.h"
#include <nds.h>
#include "tool/dbgtool.h"

SystemFilenames::SystemFilenames()
{
    if (!sys().useNitroFS())
    {
        _uiDirectory = SFN_SYSTEM_UI_DIRECTORY;
    }
    else
    {
        if (access(SFN_SYSTEM_UI_DIRECTORY "/uisettings.ini", F_OK) == 0)
        {
            _uiDirectory = SFN_SYSTEM_UI_DIRECTORY;
        }
        else
        {  
            dbg_printf("FALLBACK REQUIRED\n");
            cwl();
            _uiDirectory = SFN_FALLBACK_UI_DIRECTORY;
        }
    }
}