#include "systemfilenames.h"
#include "common/systemdetails.h"
#include <nds.h>
#include "tool/dbgtool.h"
#include "tool/stringtool.h"
#include "common/dsimenusettings.h"

SystemFilenames::SystemFilenames()
{
    std::string systemDirectory = formatString(SFN_SYSTEM_UI_DIRECTORY "%s", ms().ak_theme.c_str());
    if (!sys().useNitroFS())
    {
        // nocashMessage("NNONITROFS");
        // nocashMessage(systemDirectory.c_str());
        _uiDirectory = systemDirectory;
    }
    else
    {
        if (access((systemDirectory + "/uisettings.ini").c_str(), F_OK) == 0)
        {
            // nocashMessage("NITROFSOK");
            // nocashMessage(systemDirectory.c_str());
            _uiDirectory = systemDirectory;
        }
        else
        {  
            cwl();
            _uiDirectory = SFN_FALLBACK_UI_DIRECTORY;
        }
    }
}