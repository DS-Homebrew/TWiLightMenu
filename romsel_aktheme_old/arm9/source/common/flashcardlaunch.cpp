#include <nds/arm9/dldi.h>

#include "loaderconfig.h"
#include "flashcardlaunch.h"
#include "pergamesettings.h"
#include "tool/stringtool.h"
#include "dsimenusettings.h"

#define FC_PREFIX_SLASH "/"
#define FC_PREFIX_FAT "fat:/"
#define FC_PREFIX_FAT0 "fat0:/"
#define FC_PREFIX_FAT1 "fat1:/"

int loadGameOnFlashcard (std::string ndsPath, bool usePerGameSettings) {
	bool runNds_boostCpu = false;
	bool runNds_boostVram = false;
	if ((REG_SCFG_EXT != 0) && usePerGameSettings) {
		std::string filename = ndsPath;

		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx) {
			filename.erase(0, last_slash_idx + 1);
		}

		PerGameSettings gameConfig(filename);

		runNds_boostCpu = gameConfig.boostCpu == -1 ? DEFAULT_BOOST_CPU : gameConfig.boostCpu;
		runNds_boostVram = gameConfig.boostVram == -1 ? DEFAULT_BOOST_VRAM : gameConfig.boostVram;
	}

	std::string launchPath;
	if ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4TF", 4) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4iTT", 5) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0)
     || (memcmp(io_dldi_data->friendlyName, "Ace3DS+", 7) == 0)) {
		LoaderConfig config("fat:/Wfwd.dat", "fat:/_wfwd/lastsave.ini");
		launchPath = replaceAll(ndsPath.c_str(), FC_PREFIX_FAT, FC_PREFIX_FAT0);
		config.option("Save Info", "lastLoaded", launchPath);
		return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
	} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
		LoaderConfig config("fat:/_dstwo/autoboot.nds", "fat:/_dstwo/autoboot.ini");
		launchPath = replaceAll(ndsPath.c_str(), FC_PREFIX_FAT, FC_PREFIX_FAT1);
		config.option("Dir Info", "fullName", launchPath);
		return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
	} else if ((memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0)) {
		LoaderConfig config("fat:/YSMenu.nds", "fat:/TTMenu/YSMenu.ini");
		launchPath = replaceAll(ndsPath.c_str(), FC_PREFIX_FAT, FC_PREFIX_SLASH);
		config.option("YSMENU", "AUTO_BOOT", launchPath);
		return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
	}

	return 100;
}