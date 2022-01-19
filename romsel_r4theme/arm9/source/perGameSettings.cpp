#include "perGameSettings.h"
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <string>
#include <sstream>
#include <stdio.h>
#include <dirent.h>

#include <nds.h>
#include <nds/arm9/dldi.h>
#include <gl2d.h>

#include "sound.h"
#include "date.h"

#include "ndsheaderbanner.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "graphics/FontGraphic.h"
#include "graphics/TextPane.h"
#include "common/tonccpy.h"
#include "SwitchState.h"
#include "cheat.h"
#include "errorScreen.h"

#include "gbaswitch.h"
#include "nds_loader_arm9.h"

#include "myDSiMode.h"
#include "common/inifile.h"
#include "flashcard.h"

#include "twlClockExcludeMap.h"
#include "dmaExcludeMap.h"
#include "asyncReadExcludeMap.h"
#include "swiHaltHookIncludeMap.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10

extern bool useTwlCfg;

extern const char *bootstrapinipath;

extern bool arm7SCFGLocked;
extern bool isRegularDS;
extern int consoleModel;
extern bool macroMode;
extern bool lcdSwapped;

extern bool useBootstrap;
extern bool dsiWareBooter;
extern int b4dsMode;
extern int theme;

extern std::string romfolder[2];

const char* SDKnumbertext;

extern bool showdialogbox;
extern int dialogboxHeight;

extern int ignoreBlacklists;

bool perGameSettingsChanged = false;

int perGameSettings_cursorPosition = 0;
bool perGameSettings_directBoot = false;	// Homebrew only
int perGameSettings_dsiMode = -1;
int perGameSettings_language = -2;
int perGameSettings_region = -2;
int perGameSettings_saveNo = 0;
int perGameSettings_ramDiskNo = -1;
int perGameSettings_boostCpu = -1;
int perGameSettings_boostVram = -1;
int perGameSettings_cardReadDMA = -1;
int perGameSettings_asyncCardRead = -1;
int perGameSettings_swiHaltHook = -1;
int perGameSettings_bootstrapFile = -1;
int perGameSettings_wideScreen = -1;
int perGameSettings_expandRomSpace = -1;

static char SET_AS_DONOR_ROM[32];

char pergamefilepath[256];

extern void RemoveTrailingSlashes(std::string &path);

extern char usernameRendered[10];
extern bool usernameRenderedDone;

char fileCounter[8];
char gameTIDText[16];

int firstPerGameOpShown = 0;
int perGameOps = -1;
int perGameOp[10] = {-1};

bool blacklisted_boostCpu = false;
bool blacklisted_cardReadDma = false;
bool blacklisted_asyncCardRead = false;
bool whitelisted_swiHaltHook = false;

void loadPerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_directBoot = pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", (isModernHomebrew || previousUsedDevice));	// Homebrew only
	if (isHomebrew) {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", (isModernHomebrew ? true : false));
	} else {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", -1);
	}
	perGameSettings_language = pergameini.GetInt("GAMESETTINGS", "LANGUAGE", -2);
	perGameSettings_region = pergameini.GetInt("GAMESETTINGS", "REGION", -2);
	if (perGameSettings_region < -2 || (!dsiFeatures() && perGameSettings_region == -1)) perGameSettings_region = -2;
	perGameSettings_saveNo = pergameini.GetInt("GAMESETTINGS", "SAVE_NUMBER", 0);
	perGameSettings_ramDiskNo = pergameini.GetInt("GAMESETTINGS", "RAM_DISK", -1);
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
	perGameSettings_boostVram = pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", -1);
	perGameSettings_cardReadDMA = pergameini.GetInt("GAMESETTINGS", "CARD_READ_DMA", -1);
	perGameSettings_asyncCardRead = pergameini.GetInt("GAMESETTINGS", "ASYNC_CARD_READ", -1);
	perGameSettings_swiHaltHook = pergameini.GetInt("GAMESETTINGS", "SWI_HALT_HOOK", -1);
	perGameSettings_bootstrapFile = pergameini.GetInt("GAMESETTINGS", "BOOTSTRAP_FILE", -1);
	perGameSettings_wideScreen = pergameini.GetInt("GAMESETTINGS", "WIDESCREEN", -1);
	perGameSettings_expandRomSpace = pergameini.GetInt("GAMESETTINGS", "EXTENDED_MEMORY", -1);
}

void savePerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (isHomebrew) {
		if (!secondaryDevice) pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
		if (isModernHomebrew) {
			pergameini.SetInt("GAMESETTINGS", "REGION", perGameSettings_region);
		}
		if (!secondaryDevice) pergameini.SetInt("GAMESETTINGS", "RAM_DISK", perGameSettings_ramDiskNo);
		pergameini.SetInt("GAMESETTINGS", "DIRECT_BOOT", perGameSettings_directBoot);
		if (isDSiMode() || !secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		}
		if (dsiFeatures()) {
			pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		}
		if (!secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", perGameSettings_bootstrapFile);
		}
		if (dsiFeatures() && consoleModel >= 2 && sdFound()) {
			pergameini.SetInt("GAMESETTINGS", "WIDESCREEN", perGameSettings_wideScreen);
		}
	} else {
		if (useBootstrap || isDSiWare || !secondaryDevice) pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
		if ((dsiFeatures() && useBootstrap) || !secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "REGION", perGameSettings_region);
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		} else if (isDSiWare) {
			pergameini.SetInt("GAMESETTINGS", "REGION", perGameSettings_region);
		}
		pergameini.SetInt("GAMESETTINGS", "SAVE_NUMBER", perGameSettings_saveNo);
		if (dsiFeatures()) {
			if (!blacklisted_boostCpu) pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
			pergameini.SetInt("GAMESETTINGS", "SWI_HALT_HOOK", perGameSettings_swiHaltHook);
		}
		if (!secondaryDevice) {
			if (!blacklisted_cardReadDma) pergameini.SetInt("GAMESETTINGS", "CARD_READ_DMA", perGameSettings_cardReadDMA);
			if (!blacklisted_asyncCardRead) pergameini.SetInt("GAMESETTINGS", "ASYNC_CARD_READ", perGameSettings_asyncCardRead);
		}
		if (useBootstrap || isDSiWare || !secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", perGameSettings_bootstrapFile);
		}
		if (dsiFeatures() && consoleModel >= 2 && sdFound()) {
			pergameini.SetInt("GAMESETTINGS", "WIDESCREEN", perGameSettings_wideScreen);
		}
		if ((dsiFeatures() && useBootstrap) || !secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "EXTENDED_MEMORY", perGameSettings_expandRomSpace);
		}
	}
	pergameini.SaveIniFile( pergamefilepath );
}

bool checkIfShowAPMsg (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (pergameini.GetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 0) == 0) {
		return true;	// Show AP message
	}
	return false;	// Don't show AP message
}

void dontShowAPMsgAgain (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	pergameini.SetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 1);
	pergameini.SaveIniFile( pergamefilepath );
}

bool checkIfShowRAMLimitMsg (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (pergameini.GetInt("GAMESETTINGS", "NO_SHOW_RAM_LIMIT", 0) == 0) {
		return true;	// Show RAM Limit message
	}
	return false;	// Don't show RAM Limit message
}

void dontShowRAMLimitMsgAgain (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	pergameini.SetInt("GAMESETTINGS", "NO_SHOW_RAM_LIMIT_MSG", 1);
	pergameini.SaveIniFile( pergamefilepath );
}

bool checkIfDSiMode (std::string filename) {
	if (secondaryDevice && (!dsiFeatures() || !useBootstrap)) {
		return false;
	}

	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", (isModernHomebrew ? true : -1));
	if (perGameSettings_dsiMode == -1) {
		return DEFAULT_DSI_MODE;
	} else {
		return perGameSettings_dsiMode;
	}
}

bool showSetDonorRom(u32 arm7size, u32 SDKVersion, bool dsiBinariesFound) {
	if (requiresDonorRom) return false;

	bool usingB4DS = (!dsiFeatures() && secondaryDevice);
	bool dsiEnhancedMbk = (isDSiMode() && *(u32*)0x02FFE1A0 == 0x00403000 && arm7SCFGLocked);

	if ((usingB4DS || (dsiEnhancedMbk && dsiBinariesFound)) && SDKVersion > 0x5000000	// SDK5 (TWL)
	 && (arm7size==0x22B40
	  || arm7size==0x22BCC
	  || arm7size==0x28F84
	  || arm7size==0x2909C
	  || arm7size==0x2914C
	  || arm7size==0x29164
	  || arm7size==0x29EE8
	  || arm7size==0x2A2EC
	  || arm7size==0x2A318
	  || arm7size==0x2AF18
	  || arm7size==0x2B184
	  || arm7size==0x2B24C
	  || arm7size==0x2C5B4))
	{
		return true;
	}

	return false;
}

bool showSetDonorRomDSiWare(u32 arm7size) {
	if (requiresDonorRom || !isDSiMode() || *(u32*)0x02FFE1A0 == 0x00403000 || !arm7SCFGLocked) return false;

	if (arm7size==0x1D43C
	 || arm7size==0x1D5A8
	 || arm7size==0x1E1E8
	 || arm7size==0x1E22C
	 || arm7size==0x25664
	 || arm7size==0x257DC
	 || arm7size==0x25860
	 || arm7size==0x268DC
	 || arm7size==0x26BA8
	 || arm7size==0x26C5C
	 || arm7size==0x26CC8
	 || arm7size==0x26D10
	 || arm7size==0x26D48
	 || arm7size==0x26D50
	 || arm7size==0x26DF4
	 || arm7size==0x27FB4
	 || arm7size==0x28E54)
	{
		return true;
	}

	return false;
}

bool donorRomTextShown = false;
void revertDonorRomText(void) {
	if (!donorRomTextShown || strncmp(SET_AS_DONOR_ROM, "Done!", 5) != 0) return;

	sprintf(SET_AS_DONOR_ROM, "Set as Donor ROM");
}

const char* getRegionString(char region) {
	u8 twlCfgCountry = 0;
	if (useTwlCfg) {
		twlCfgCountry = *(u8*)0x02000405;
	}

	switch (region) {
		case 'C':
			return "CHN";
		case 'D':
			return "DET";
		case 'E':
		case 'L':
		case 'O':
			return "USA";
		case 'F':
			return "FRE";
		case 'H':
			return "DUT";
		case 'I':
			return "ITA";
		case 'J':
			return "JPN";
		case 'K':
			return "KOR";
		case 'M':
			return "SWE";
		case 'N':
			return "NOR";
		case 'P':
		case 'W':
		case 'X':
		case 'Y':
		case 'Z':
			return "EUR";
		case 'Q':
			return "DAN";
		case 'R':
			return "RUS";
		case 'S':
			return "SPA";
		case 'T':
			return (twlCfgCountry == 0x41 || twlCfgCountry == 0x5F) ? "AUS" : "USA";
		case 'U':
			return "AUS";
		case 'V':
			return (twlCfgCountry == 0x41 || twlCfgCountry == 0x5F) ? "AUS" : "EUR";
	}
	return "N/A";
}

void perGameSettings (std::string filename) {
	if (macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}

	int pressed = 0, held = 0;

	keysSetRepeat(25, 5); // Slow down key repeat

	clearText();
	
	perGameSettings_cursorPosition = 0;
	loadPerGameSettings(filename);

	std::string filenameForInfo = filename;
	if((filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "argv")
	|| (filenameForInfo.substr(filenameForInfo.find_last_of(".") + 1) == "ARGV"))
	{
		std::vector<char*> argarray;

		FILE *argfile = fopen(filenameForInfo.c_str(),"rb");
			char str[PATH_MAX], *pstr;
		const char seps[]= "\n\r\t ";

		while( fgets(str, PATH_MAX, argfile) ) {
			// Find comment and end string there
			if( (pstr = strchr(str, '#')) )
				*pstr= '\0';

			// Tokenize arguments
			pstr= strtok(str, seps);

			while( pstr != NULL ) {
				argarray.push_back(strdup(pstr));
				pstr= strtok(NULL, seps);
			}
		}
		fclose(argfile);
		filenameForInfo = argarray.at(0);
	}

	FILE *f_nds_file = fopen(filenameForInfo.c_str(), "rb");

	char game_TID[5];
	grabTID(f_nds_file, game_TID);
	game_TID[4] = 0;

	// Check if blacklisted
	blacklisted_boostCpu = false;
	blacklisted_cardReadDma = false;
	blacklisted_asyncCardRead = false;
	whitelisted_swiHaltHook = false;
	if(!ignoreBlacklists) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
			if (memcmp(game_TID, twlClockExcludeList[i], 3) == 0) {
				// Found match
				blacklisted_boostCpu = true;
			}
		}

		for (unsigned int i = 0; i < sizeof(cardReadDMAExcludeList)/sizeof(cardReadDMAExcludeList[0]); i++) {
			if (memcmp(game_TID, cardReadDMAExcludeList[i], 3) == 0) {
				// Found match
				blacklisted_cardReadDma = true;
			}
		}

		for (unsigned int i = 0; i < sizeof(asyncReadExcludeList)/sizeof(asyncReadExcludeList[0]); i++) {
			if (memcmp(game_TID, asyncReadExcludeList[i], 3) == 0) {
				// Found match
				blacklisted_asyncCardRead = true;
			}
		}

		for (unsigned int i = 0; i < sizeof(swiHaltHookIncludeList)/sizeof(swiHaltHookIncludeList[0]); i++) {
			if (memcmp(game_TID, swiHaltHookIncludeList[i], 3) == 0) {
				// Found match
				whitelisted_swiHaltHook = true;
			}
		}
	}

	bool showSDKVersion = false;
	u32 SDKVersion = 0;
	u8 sdkSubVer = 0;
	char sdkSubVerChar[8] = {0};
	if (memcmp(game_TID, "HND", 3) == 0 || memcmp(game_TID, "HNE", 3) == 0 || !isHomebrew) {
		SDKVersion = getSDKVersion(f_nds_file);
		tonccpy(&sdkSubVer, (u8*)&SDKVersion+2, 1);
		sprintf(sdkSubVerChar, "%d", sdkSubVer);
		showSDKVersion = true;
	}
	u32 arm7size = 0;
	u32 romSize = 0;
	u32 pubSize = 0;
	u32 prvSize = 0;
	fseek(f_nds_file, 0x3C, SEEK_SET);
	fread(&arm7size, sizeof(u32), 1, f_nds_file);
	fseek(f_nds_file, 0x80, SEEK_SET);
	fread(&romSize, sizeof(u32), 1, f_nds_file);
	fseek(f_nds_file, 0x238, SEEK_SET);
	fread(&pubSize, sizeof(u32), 1, f_nds_file);
	fread(&prvSize, sizeof(u32), 1, f_nds_file);
	bool dsiBinariesFound = checkDsiBinaries(f_nds_file);
	fclose(f_nds_file);

	if (romSize > 0) {
		romSize -= 0x8000;	// First 32KB
		romSize += 0x88;	// RSA key
	}

	u32 romSizeLimit = (consoleModel > 0 ? 0x01800000 : 0x800000);
	if (SDKVersion > 0x5000000) {
		romSizeLimit = (consoleModel > 0 ? 0x01000000 : 0);
	}
	u32 romSizeLimit2 = (consoleModel > 0 ? 0x01C00000 : 0xC00000);

	extern bool dsiWareCompatibleB4DS(const char* filename);
	bool showPerGameSettings =
		(!isDSiWare
		&& memcmp(game_TID, "HND", 3) != 0
		&& memcmp(game_TID, "HNE", 3) != 0);
	if ((dsiFeatures() || dsiWareCompatibleB4DS(filenameForInfo.c_str()) || !secondaryDevice) && !isHomebrew && isDSiWare) {
		showPerGameSettings = true;
	}
	/*if (!useBootstrap && !isHomebrew && REG_SCFG_EXT == 0) {
		showPerGameSettings = false;
	}*/
	bool runInShown = false;

	bool showCheats = ((useBootstrap
	/*|| (secondaryDevice && !useBootstrap
		&& ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
		 || (memcmp(io_dldi_data->friendlyName, "R4TF", 4) == 0)
		 || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
		 || (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0)))*/
	|| !secondaryDevice) && !isHomebrew && !isDSiWare
	&& memcmp(game_TID, "HND", 3) != 0
	&& memcmp(game_TID, "HNE", 3) != 0);

	firstPerGameOpShown = 0;
	perGameOps = -1;
	for (int i = 0; i < 10; i++) {
		perGameOp[i] = -1;
	}
	if (isHomebrew) {		// Per-game settings for homebrew
		perGameOps++;
		perGameOp[perGameOps] = 0;	// Language
		if (isModernHomebrew) {
			perGameOps++;
			perGameOp[perGameOps] = 11;	// Region
		}
		if (!secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 1;	// RAM disk number
		}
		perGameOps++;
		perGameOp[perGameOps] = 6;	// Direct boot
		if (isDSiMode() || !secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 2;	// Run in
			runInShown = true;
		}
		if (dsiFeatures() || !secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 3;	// ARM9 CPU Speed
			perGameOps++;
			perGameOp[perGameOps] = 4;	// VRAM Boost
		}
		if (!secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 7;	// Bootstrap
		}
		if (dsiFeatures() && consoleModel >= 2 && sdFound() && (game_TID[0] == 'W' || romVersion == 0x57)) {
			perGameOps++;
			perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
		}
	} else if (showPerGameSettings && isDSiWare) {	// Per-game settings for DSiWare
		if (dsiWareBooter || arm7SCFGLocked || consoleModel > 0) {
			perGameOps++;
			perGameOp[perGameOps] = 0;	// Language
			perGameOps++;
			perGameOp[perGameOps] = 11;	// Region
		}
		if (pubSize > 0 || prvSize > 0) {
			perGameOps++;
			perGameOp[perGameOps] = 1;	// Save number
		}
		if (dsiWareBooter || arm7SCFGLocked || consoleModel > 0) {
			perGameOps++;
			perGameOp[perGameOps] = 7;	// Bootstrap
			if (((dsiFeatures() && sdFound()) || !secondaryDevice) && consoleModel >= 2 && (!isDSiMode() || !arm7SCFGLocked)) {
				perGameOps++;
				perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
			}
			showCheats = true;
		}
		if (a7mbk6 == 0x080037C0 ? showSetDonorRomDSiWare(arm7size) : showSetDonorRom(arm7size, SDKVersion, dsiBinariesFound)) {
			perGameOps++;
			perGameOp[perGameOps] = 9;	// Set as Donor ROM
			donorRomTextShown = true;
		} else {
			donorRomTextShown = false;
		}
	} else if (showPerGameSettings) {	// Per-game settings for retail/commercial games
		if (useBootstrap || (dsiFeatures() && romUnitCode > 0) || !secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 0;	// Language
		}
		if (((dsiFeatures() && useBootstrap) || !secondaryDevice) && romUnitCode == 3) {
			perGameOps++;
			perGameOp[perGameOps] = 11;	// Region
		}
		perGameOps++;
		perGameOp[perGameOps] = 1;	// Save number
		if ((dsiFeatures() && (useBootstrap || romUnitCode > 0) && !b4dsMode) || !secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 2;	// Run in
			runInShown = true;
		}
		if ((dsiFeatures() || !secondaryDevice) && romUnitCode < 3) {
			if (!blacklisted_boostCpu) {
				perGameOps++;
				perGameOp[perGameOps] = 3;	// ARM9 CPU Speed
			}
			perGameOps++;
			perGameOp[perGameOps] = 4;	// VRAM Boost
		}
		if (useBootstrap || (dsiFeatures() && romUnitCode > 0) || !secondaryDevice) {
			if (!secondaryDevice) {
				if (romUnitCode < 3 && !blacklisted_cardReadDma) {
					perGameOps++;
					perGameOp[perGameOps] = 5;	// Card Read DMA
				}
				if (romSize > romSizeLimit && !blacklisted_asyncCardRead) {
					perGameOps++;
					perGameOp[perGameOps] = 12;	// Async Card Read
				}
			}
			if ((!secondaryDevice || (dsiFeatures() && romSize <= romSizeLimit2+0x80000)) && romUnitCode != 3 && !whitelisted_swiHaltHook) {
				perGameOps++;
				perGameOp[perGameOps] = 13;	// SWI Halt Hook
			}
			if ((dsiFeatures() || !secondaryDevice)
			 && romSize > romSizeLimit && romSize <= romSizeLimit2+0x80000) {
				perGameOps++;
				perGameOp[perGameOps] = 10;	// Expand ROM space in RAM
			}
			perGameOps++;
			perGameOp[perGameOps] = 7;	// Bootstrap
			if (((dsiFeatures() && sdFound()) || !secondaryDevice) && consoleModel >= 2 && (!isDSiMode() || !arm7SCFGLocked)) {
				perGameOps++;
				perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
			}
			if (a7mbk6 == 0x080037C0 ? showSetDonorRomDSiWare(arm7size) : showSetDonorRom(arm7size, SDKVersion, dsiBinariesFound)) {
				perGameOps++;
				perGameOp[perGameOps] = 9;	// Set as Donor ROM
				donorRomTextShown = true;
			} else {
				donorRomTextShown = false;
			}
		}
	}

	if (isHomebrew) {
		snprintf (gameTIDText, sizeof(gameTIDText), game_TID[0]==0 ? "" : "TID: %s", game_TID);
	} else {
		snprintf (gameTIDText, sizeof(gameTIDText), "%s-%s-%s", romUnitCode > 0 ? "TWL" : "NTR", game_TID, getRegionString(game_TID[3]));
	}

	char saveNoDisplay[16];

	if((SDKVersion > 0x1000000) && (SDKVersion < 0x2000000)) {
		SDKnumbertext = ("SDK ver: 1."+(std::string)sdkSubVerChar).c_str();
	} else if((SDKVersion > 0x2000000) && (SDKVersion < 0x3000000)) {
		SDKnumbertext = ("SDK ver: 2."+(std::string)sdkSubVerChar).c_str();
	} else if((SDKVersion > 0x3000000) && (SDKVersion < 0x4000000)) {
		SDKnumbertext = ("SDK ver: 3."+(std::string)sdkSubVerChar).c_str();
	} else if((SDKVersion > 0x4000000) && (SDKVersion < 0x5000000)) {
		SDKnumbertext = ("SDK ver: 4."+(std::string)sdkSubVerChar).c_str();
	} else if((SDKVersion > 0x5000000) && (SDKVersion < 0x6000000)) {
		SDKnumbertext = ("SDK ver: 5."+(std::string)sdkSubVerChar).c_str();
	} else {
		SDKnumbertext = "SDK ver: ???";
	}
	if (!showPerGameSettings) {
		dialogboxHeight = 0;
	} else {
		dialogboxHeight = 4;
	}

	sprintf(SET_AS_DONOR_ROM, "Set as Donor ROM");

	showdialogbox = true;

	while (1) {
		clearText();
		if (theme == 6) {
			printLarge(false, 0, 56, filename.c_str());
		} else {
			titleUpdate(isDirectory, filename.c_str());
		}

		printLargeCentered(false, 74, showPerGameSettings ? "Game settings" : "Info");
		if (showSDKVersion) printSmall(false, 24, 90, SDKnumbertext);
		printSmallRightAlign(false, 232, 90, gameTIDText);

		int perGameOpYpos = 102;
		bool flashcardKernelOnly = (!useBootstrap && secondaryDevice && !isHomebrew && romUnitCode == 2 && (perGameSettings_dsiMode==-1 ? !DEFAULT_DSI_MODE : perGameSettings_dsiMode==0));

		if (showPerGameSettings) {
			printSmall(false, 24, 102+(perGameSettings_cursorPosition*12)-(firstPerGameOpShown*12), ">");
		}

		for (int i = firstPerGameOpShown; i < firstPerGameOpShown+4; i++) {
		if (!showPerGameSettings || perGameOp[i] == -1) break;
		switch (perGameOp[i]) {
			case 0:
				printSmall(false, 32, perGameOpYpos, "Language:");
				if (perGameSettings_language == -1 || flashcardKernelOnly) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "System");
				} else if (perGameSettings_language == -2) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
				} else if (perGameSettings_language == 0) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Japanese");
				} else if (perGameSettings_language == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "English");
				} else if (perGameSettings_language == 2) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "French");
				} else if (perGameSettings_language == 3) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "German");
				} else if (perGameSettings_language == 4) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Italian");
				} else if (perGameSettings_language == 5) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Spanish");
				} else if (perGameSettings_language == 6) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Chinese");
				} else if (perGameSettings_language == 7) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Korean");
				}
				break;
			case 1:
				if (isHomebrew) {
					printSmall(false, 32, perGameOpYpos, "RAM disk:");
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "%i", perGameSettings_ramDiskNo);
				} else {
					printSmall(false, 32, perGameOpYpos, "Save Number:");
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "%i", perGameSettings_saveNo);
				}
				if (isHomebrew && perGameSettings_ramDiskNo == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "None");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, saveNoDisplay);
				}
				break;
			case 2:
				printSmall(false, 32, perGameOpYpos, "Run in:");
				if (perGameSettings_dsiMode == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
				} else if (perGameSettings_dsiMode > 0) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "DSi mode");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "DS mode");
				}
				break;
			case 3:
				printSmall(false, 32, perGameOpYpos, "ARM9 CPU Speed:");
				if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE && romUnitCode > 0) : perGameSettings_dsiMode > 0) && runInShown) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "133mhz (TWL)");
				} else {
					if (perGameSettings_boostCpu == -1) {
						printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
					} else if (perGameSettings_boostCpu == 1) {
						printSmallRightAlign(false, 256-24, perGameOpYpos, "133mhz (TWL)");
					} else {
						printSmallRightAlign(false, 256-24, perGameOpYpos, "67mhz (NTR)");
					}
				}
				break;
			case 4:
				printSmall(false, 32, perGameOpYpos, "VRAM Mode:");
				if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE && romUnitCode > 0) : perGameSettings_dsiMode > 0) && runInShown) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, romUnitCode == 0 ? "DSi mode" : "Auto");
				} else {
					if (perGameSettings_boostVram == -1) {
						printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
					} else if (perGameSettings_boostVram == 1) {
						printSmallRightAlign(false, 256-24, perGameOpYpos, "DSi mode");
					} else {
						printSmallRightAlign(false, 256-24, perGameOpYpos, "DS mode");
					}
				}
				break;
			case 5:
				printSmall(false, 32, perGameOpYpos, "Card Read DMA:");
				if (perGameSettings_cardReadDMA == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
				} else if (perGameSettings_cardReadDMA == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "On");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Off");
				}
				break;
			case 6:
				printSmall(false, 32, perGameOpYpos, "Direct Boot:");
				if (perGameSettings_directBoot) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Yes");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "No");
				}
				break;
			case 7:
				printSmall(false, 32, perGameOpYpos, "Bootstrap:");
				if (flashcardKernelOnly) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Not Used");
				} else if (perGameSettings_bootstrapFile == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
				} else if (perGameSettings_bootstrapFile == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Nightly");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Release");
				}
				break;
			case 8:
				printSmall(false, 32, perGameOpYpos, "Screen Aspect Ratio:");
				if (flashcardKernelOnly) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Not Used");
				} else if (perGameSettings_wideScreen == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
				} else if (perGameSettings_wideScreen == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "16:10");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "4:3");
				}
				break;
			case 9:
				printSmallCentered(false, perGameOpYpos, SET_AS_DONOR_ROM);
				break;
			case 10:
				printSmall(false, 32, perGameOpYpos, "Ex. ROM space in RAM:");
				if (flashcardKernelOnly) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Not Used");
				} else if (perGameSettings_expandRomSpace == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
				} else if (perGameSettings_expandRomSpace == 2) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Yes+512KB");
				} else if (perGameSettings_expandRomSpace == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Yes");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "No");
				}
				break;
			case 11:
				printSmall(false, 32, perGameOpYpos, "Region:");
				if (perGameSettings_region == -2) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
				} else if (perGameSettings_region == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "System");
				} else if (perGameSettings_region == 0) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Japan");
				} else if (perGameSettings_region == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "USA");
				} else if (perGameSettings_region == 2) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Europe");
				} else if (perGameSettings_region == 3) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Australia");
				} else if (perGameSettings_region == 4) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "China");
				} else if (perGameSettings_region == 5) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Korea");
				}
				break;
			case 12:
				printSmall(false, 32, perGameOpYpos, "Asynch Card Read:");
				if (perGameSettings_asyncCardRead == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
				} else if (perGameSettings_asyncCardRead == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "On");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Off");
				}
				break;
			case 13:
				printSmall(false, 32, perGameOpYpos, "SWI Halt Hook:");
				if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE && romUnitCode > 0) : perGameSettings_dsiMode > 0) && runInShown) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Off");
				} else if (perGameSettings_swiHaltHook == -1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Default");
				} else if (perGameSettings_swiHaltHook == 1) {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "On");
				} else {
					printSmallRightAlign(false, 256-24, perGameOpYpos, "Off");
				}
				break;
		}
		perGameOpYpos += 12;
		}
		if (!showPerGameSettings && !showCheats) {
			printSmallCentered(false, perGameOpYpos+6, "\u2427 OK");
		} else {
			printSmallCentered(false, perGameOpYpos+6, showCheats ? "\u2429 Cheats  \u2428 Back" : "\u2428 Back");
		}
		do {
			snd().updateStream();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
			checkSdEject();
			swiWaitForVBlank();
		} while (!held);

		if (!showPerGameSettings) {
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				break;
			}
		} else {
			if (held & KEY_UP) {
				revertDonorRomText();
				perGameSettings_cursorPosition--;
				if (perGameSettings_cursorPosition < 0) {
					perGameSettings_cursorPosition = perGameOps;
					firstPerGameOpShown = (perGameOps>2 ? perGameOps-3 : 0);
				} else if (perGameSettings_cursorPosition < firstPerGameOpShown) {
					firstPerGameOpShown--;
				}
			}
			if (held & KEY_DOWN) {
				revertDonorRomText();
				perGameSettings_cursorPosition++;
				if (perGameSettings_cursorPosition > perGameOps) {
					perGameSettings_cursorPosition = 0;
					firstPerGameOpShown = 0;
				} else if (perGameSettings_cursorPosition > firstPerGameOpShown+3) {
					firstPerGameOpShown++;
				}
			}

			if (held & KEY_LEFT) {
				switch (perGameOp[perGameSettings_cursorPosition]) {
					case 0:
						if (!flashcardKernelOnly) {
							perGameSettings_language--;
							if (perGameSettings_language < -2) perGameSettings_language = 7;
						}
						break;
					case 1:
						if (isHomebrew) {
							if (!perGameSettings_directBoot) {
								perGameSettings_ramDiskNo--;
								if (perGameSettings_ramDiskNo < -1) perGameSettings_ramDiskNo = 9;
							}
						} else {
							perGameSettings_saveNo--;
							if (perGameSettings_saveNo < 0) perGameSettings_saveNo = 9;
						}
						break;
					case 2:
						perGameSettings_dsiMode--;
						if (!isHomebrew) {
							if ((perGameSettings_dsiMode == 1 && romUnitCode == 0)
							 || (perGameSettings_dsiMode == 2 && romUnitCode > 0)) perGameSettings_dsiMode--;
						}
						if (perGameSettings_dsiMode < -1) perGameSettings_dsiMode = 2-isHomebrew;
						if (!isHomebrew) {
							if (perGameSettings_dsiMode == 2 && romUnitCode > 0) perGameSettings_dsiMode--;
						}
						break;
					case 3:
						if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE == 0 || romUnitCode == 0) : perGameSettings_dsiMode < 1) || !runInShown) {
							perGameSettings_boostCpu--;
							if (perGameSettings_boostCpu < -1) perGameSettings_boostCpu = 1;
						}
						break;
					case 4:
						if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE == 0 || romUnitCode == 0) : perGameSettings_dsiMode < 1) || !runInShown) {
							perGameSettings_boostVram--;
							if (perGameSettings_boostVram < -1) perGameSettings_boostVram = 1;
						}
						break;
					case 5:
						perGameSettings_cardReadDMA--;
						if (perGameSettings_cardReadDMA < -1) perGameSettings_cardReadDMA = 1;
						break;
					case 6:
						perGameSettings_directBoot = !perGameSettings_directBoot;
						break;
					case 7:
						if (!flashcardKernelOnly) {
							perGameSettings_bootstrapFile--;
							if (perGameSettings_bootstrapFile < -1) perGameSettings_bootstrapFile = 1;
						}
						break;
					case 8:
						perGameSettings_wideScreen++;
						if (perGameSettings_wideScreen > 1) perGameSettings_wideScreen = -1;
						break;
					case 10:
						perGameSettings_expandRomSpace--;
						if (perGameSettings_expandRomSpace==1 && romSize > romSizeLimit2) {
							perGameSettings_expandRomSpace--;
						} else if (perGameSettings_expandRomSpace==2 && romSize <= romSizeLimit2) {
							perGameSettings_expandRomSpace--;
						}
						if (perGameSettings_expandRomSpace < -1) perGameSettings_expandRomSpace = 2;
						break;
					case 11:
						perGameSettings_region--;
						if (!dsiFeatures() && perGameSettings_region == -1) {
							perGameSettings_region--;
						}
						if (perGameSettings_region < -2) perGameSettings_region = 5;
						break;
					case 12:
						perGameSettings_asyncCardRead--;
						if (perGameSettings_asyncCardRead < -1) perGameSettings_asyncCardRead = 1;
						break;
					case 13:
						if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE == 0 || romUnitCode == 0) : perGameSettings_dsiMode < 1) || !runInShown) {
							perGameSettings_swiHaltHook--;
							if (perGameSettings_swiHaltHook < -1) perGameSettings_swiHaltHook = 1;
						}
						break;
				}
				perGameSettingsChanged = true;
			} else if ((pressed & KEY_A) || (held & KEY_RIGHT)) {
				switch (perGameOp[perGameSettings_cursorPosition]) {
					case 0:
						if (!flashcardKernelOnly) {
							perGameSettings_language++;
							if (perGameSettings_language > 7) perGameSettings_language = -2;
						}
						break;
					case 1:
						if (isHomebrew) {
							if (!perGameSettings_directBoot) {
								perGameSettings_ramDiskNo++;
								if (perGameSettings_ramDiskNo > 9) perGameSettings_ramDiskNo = -1;
							}
						} else {
							perGameSettings_saveNo++;
							if (perGameSettings_saveNo > 9) perGameSettings_saveNo = 0;
						}
						break;
					case 2:
						perGameSettings_dsiMode++;
						if (!isHomebrew) {
							if ((perGameSettings_dsiMode == 1 && romUnitCode == 0)
							 || (perGameSettings_dsiMode == 2 && romUnitCode > 0)) perGameSettings_dsiMode++;
						}
						if (perGameSettings_dsiMode > 2-isHomebrew) perGameSettings_dsiMode = -1;
						break;
					case 3:
						if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE == 0 || romUnitCode == 0) : perGameSettings_dsiMode < 1) || !runInShown) {
							perGameSettings_boostCpu++;
							if (perGameSettings_boostCpu > 1) perGameSettings_boostCpu = -1;
						}
						break;
					case 4:
						if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE == 0 || romUnitCode == 0) : perGameSettings_dsiMode < 1) || !runInShown) {
							perGameSettings_boostVram++;
							if (perGameSettings_boostVram > 1) perGameSettings_boostVram = -1;
						}
						break;
					case 5:
						perGameSettings_cardReadDMA++;
						if (perGameSettings_cardReadDMA > 1) perGameSettings_cardReadDMA = -1;
						break;
					case 6:
						perGameSettings_directBoot = !perGameSettings_directBoot;
						break;
					case 7:
						if (!flashcardKernelOnly) {
							perGameSettings_bootstrapFile++;
							if (perGameSettings_bootstrapFile > 1) perGameSettings_bootstrapFile = -1;
						}
						break;
					case 8:
						perGameSettings_wideScreen++;
						if (perGameSettings_wideScreen > 1) perGameSettings_wideScreen = -1;
						break;
					case 9:
					  if (pressed & KEY_A) {
						const char* pathDefine = a7mbk6 == 0x080037C0 ? "DONORTWLONLY_NDS_PATH" : "DONORTWL_NDS_PATH"; // SDK5.x
						if (arm7size == 0x26CC8
						 || arm7size == 0x28E54
						 || arm7size == 0x29EE8)
						{
							pathDefine = a7mbk6 == 0x080037C0 ? "DONORTWLONLY0_NDS_PATH" : "DONORTWL0_NDS_PATH"; // SDK5.0
						}
						std::string romFolderNoSlash = romfolder[secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						bootstrapinipath = sdFound() ? "sd:/_nds/nds-bootstrap.ini" : "fat:/_nds/nds-bootstrap.ini";
						CIniFile bootstrapini(bootstrapinipath);
						bootstrapini.SetString("NDS-BOOTSTRAP", pathDefine, romFolderNoSlash+"/"+filename);
						bootstrapini.SaveIniFile(bootstrapinipath);
						sprintf(SET_AS_DONOR_ROM, "Done!");
					  }
						break;
					case 10:
						perGameSettings_expandRomSpace++;
						if (perGameSettings_expandRomSpace==1 && romSize > romSizeLimit2) {
							perGameSettings_expandRomSpace++;
						} else if (perGameSettings_expandRomSpace==2 && romSize <= romSizeLimit2) {
							perGameSettings_expandRomSpace++;
						}
						if (perGameSettings_expandRomSpace > 2) perGameSettings_expandRomSpace = -1;
						break;
					case 11:
						perGameSettings_region++;
						if (!dsiFeatures() && perGameSettings_region == -1) {
							perGameSettings_region++;
						}
						if (perGameSettings_region > 5) perGameSettings_region = -2;
						break;
					case 12:
						perGameSettings_asyncCardRead++;
						if (perGameSettings_asyncCardRead > 1) perGameSettings_asyncCardRead = -1;
						break;
					case 13:
						if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE == 0 || romUnitCode == 0) : perGameSettings_dsiMode < 1) || !runInShown) {
							perGameSettings_swiHaltHook++;
							if (perGameSettings_swiHaltHook > 1) perGameSettings_swiHaltHook = -1;
						}
						break;
				}
				perGameSettingsChanged = true;
			}

			if (pressed & KEY_B) {
				if (perGameSettingsChanged) {
					savePerGameSettings(filename);
					perGameSettingsChanged = false;
				}
				break;
			}
		}
		if ((pressed & KEY_X) && !isHomebrew && showCheats) {
			CheatCodelist codelist;
			codelist.selectCheats(filename);
		}
	}
	clearText();
	showdialogbox = false;
	dialogboxHeight = 0;
	keysSetRepeat(10, 2); // Reset key repeat

	if (macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}
}

std::string getSavExtension(void) {
	if (perGameSettings_saveNo == 0) {
		return ".sav";
	} else {
		return ".sav" + std::to_string(perGameSettings_saveNo);
	}
}

std::string getPubExtension(void) {
	if (perGameSettings_saveNo == 0) {
		return ".pub";
	} else {
		return ".pu" + std::to_string(perGameSettings_saveNo);
	}
}

std::string getPrvExtension(void) {
	if (perGameSettings_saveNo == 0) {
		return ".prv";
	} else {
		return ".pr" + std::to_string(perGameSettings_saveNo);
	}
}

std::string getImgExtension(int number) {
	if (number == 0) {
		return ".img";
	} else {
		return ".img" + std::to_string(number);
	}
}
