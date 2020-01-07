#include <nds/arm9/dldi.h>

#include "flashcardlaunch.h"
#include "pergamesettings.h"
#include "common/stringtool.h"

#define FC_PREFIX_SLASH "/"
#define FC_PREFIX_FAT "fat:/"
#define FC_PREFIX_FAT0 "fat0:/"
#define FC_PREFIX_FAT1 "fat1:/"

int loadGameOnFlashcard(const char *ndsPath, std::string filename, bool usePerGameSettings)
{
	bool runNds_boostCpu = false;
	bool runNds_boostVram = false;
	/*if (isDSiMode() && usePerGameSettings) {
		PerGameSettings gameConfig(filename);
		if (PerGameSettings.boostCpu == -1) {
			runNds_boostCpu = bs().boostCpu;
		} else {
			runNds_boostCpu = PerGameSettings::boostCpu;
		}
		if (PerGameSettings.boostVram == -1) {
			runNds_boostVram = bs().boostVram;
		} else {
			runNds_boostVram = PerGameSettings.boostVram;
		}
	}*/
	std::string launchPath;
	if (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0) {
		LoaderConfig config("fat:/Wfwd.dat", "fat:/_wfwd/lastsave.ini");
		launchPath = replaceAll(ndsPath, FC_PREFIX_FAT, FC_PREFIX_FAT0);
		config.option("Save Info", "lastLoaded", launchPath);
		return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
	} else if (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0) {
		LoaderConfig config("fat:/Afwd.dat", "fat:/_afwd/lastsave.ini");
		launchPath = replaceAll(ndsPath, FC_PREFIX_FAT, FC_PREFIX_FAT0);
		config.option("Save Info", "lastLoaded", launchPath);
		return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
	} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
		LoaderConfig config("fat:/_dstwo/autoboot.nds", "fat:/_dstwo/autoboot.ini");
		launchPath = replaceAll(ndsPath, FC_PREFIX_FAT, FC_PREFIX_FAT1);
		config.option("Dir Info", "fullName", launchPath);
		return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
	}

	return 100;
}