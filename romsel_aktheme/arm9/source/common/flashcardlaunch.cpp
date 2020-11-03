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

		runNds_boostCpu = gameConfig.boostCpu == -1 ? ms().boostCpu : gameConfig.boostCpu;
		runNds_boostVram = gameConfig.boostVram == -1 ? ms().boostVram : gameConfig.boostVram;
	}

	std::string launchPath;


	switch (ms().flashcard) {
		case 1: {
			LoaderConfig config("fat:/YSMenu.nds", "fat:/TTMenu/YSMenu.ini");
			launchPath = replaceAll(ndsPath.c_str(), FC_PREFIX_FAT, FC_PREFIX_SLASH);
			config.option("YSMENU", "AUTO_BOOT", launchPath);
			return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
		}
		case 6: // Blue card can run wood 1.62 so this should work?
		case 2: // And clones that can run wood (no N5)
		case 3: {
			LoaderConfig config("fat:/Wfwd.dat", "fat:/_wfwd/lastsave.ini");
			launchPath = replaceAll(ndsPath.c_str(), FC_PREFIX_FAT, FC_PREFIX_FAT0);
			config.option("Save Info", "lastLoaded", launchPath);
			return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
		}
		case 7: {
			LoaderConfig config("fat:/_dstwo/autoboot.nds", "fat:/_dstwo/autoboot.ini");
			launchPath = replaceAll(ndsPath.c_str(), FC_PREFIX_FAT, FC_PREFIX_FAT1);
			config.option("Dir Info", "fullName", launchPath);
			return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
		}
		case 5: // ?
		case 4: {
			LoaderConfig config("fat:/Afwd.dat", "fat:/_afwd/lastsave.ini");
			launchPath = replaceAll(ndsPath.c_str(), FC_PREFIX_FAT, FC_PREFIX_FAT0);
			config.option("Save Info", "lastLoaded", launchPath);
			return config.launch(0, NULL, true, true, runNds_boostCpu, runNds_boostVram);
		}
	}

	return 100;
}
