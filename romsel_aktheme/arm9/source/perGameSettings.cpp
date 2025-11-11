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

#include "date.h"

#include "ndsheaderbanner.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "graphics/FontGraphic.h"
#include "common/tonccpy.h"
#include "SwitchState.h"
#include "cheat.h"
#include "errorScreen.h"
#include "gbaswitch.h"

#include "myDSiMode.h"
#include "common/bootstrapsettings.h"
#include "common/inifile.h"
#include "common/flashcard.h"
#include "common/nds_loader_arm9.h"
#include "common/systemdetails.h"
#include "common/twlmenusettings.h"

#include "colorLutBlacklist.h"
#include "twlClockExcludeMap.h"
#include "dmaExcludeMap.h"
#include "asyncReadExcludeMap.h"

extern bool useTwlCfg;

extern u16* colorTable;
extern bool lcdSwapped;

std::string SDKnumbertext;

extern bool widescreenFound;
extern bool showdialogbox;
extern int dialogboxHeight;

extern int cursorPosOnScreen;

bool perGameSettingsChanged = false;

int perGameSettings_cursorPosition = 0;
bool perGameSettings_directBoot = false;	// Homebrew only
int perGameSettings_dsiMode = -1;
int perGameSettings_dsPhatColors = -1;
int perGameSettings_language = -2;
int perGameSettings_region = -2;
int perGameSettings_saveNo = 0;
int perGameSettings_ramDiskNo = -1;
int perGameSettings_boostCpu = -1;
int perGameSettings_boostVram = -1;
int perGameSettings_cardReadDMA = -1;
int perGameSettings_asyncCardRead = -1;
int perGameSettings_bootstrapFile = -1;
int perGameSettings_wideScreen = -1;
int perGameSettings_dsiwareBooter = -1;
int perGameSettings_useBootstrap = -1;
int perGameSettings_useBootstrapCheat = -1;
int perGameSettings_saveRelocation = -1;

static char SET_AS_DONOR_ROM[32];

char pergamefilepath[256];

extern void RemoveTrailingSlashes(std::string &path);
bool extension(const std::string_view filename, const std::vector<std::string_view> extensions);

extern void bgOperations(bool waitFrame);

extern char usernameRendered[10];
extern bool usernameRenderedDone;

char fileCounter[8];
char gameTIDText[16];

static int firstPerGameOpShown = 0;
static int perGameOps = -1;
static int perGameOp[18] = {-1};

bool blacklisted_colorLut = false;
bool blacklisted_boostCpu = false;
bool blacklisted_cardReadDma = false;
bool blacklisted_asyncCardRead = false;

void loadPerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_directBoot = pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", (isModernHomebrew[cursorPosOnScreen] || ms().previousUsedDevice));	// Homebrew only
	if (isHomebrew[cursorPosOnScreen]) {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", (isModernHomebrew[cursorPosOnScreen] ? true : false));
	} else {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", -1);
	}
	perGameSettings_dsPhatColors = pergameini.GetInt("GAMESETTINGS", "PHAT_COLORS", -1);
	perGameSettings_language = pergameini.GetInt("GAMESETTINGS", "LANGUAGE", -2);
	perGameSettings_region = pergameini.GetInt("GAMESETTINGS", "REGION", -2);
	if (perGameSettings_region < -2 || (!dsiFeatures() && perGameSettings_region == -1)) perGameSettings_region = -2;
	perGameSettings_saveNo = pergameini.GetInt("GAMESETTINGS", "SAVE_NUMBER", 0);
	perGameSettings_ramDiskNo = pergameini.GetInt("GAMESETTINGS", "RAM_DISK", -1);
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
	perGameSettings_boostVram = pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", -1);
	perGameSettings_cardReadDMA = pergameini.GetInt("GAMESETTINGS", "CARD_READ_DMA", -1);
	perGameSettings_asyncCardRead = pergameini.GetInt("GAMESETTINGS", "ASYNC_CARD_READ", -1);
	perGameSettings_bootstrapFile = pergameini.GetInt("GAMESETTINGS", "BOOTSTRAP_FILE", -1);
	perGameSettings_wideScreen = pergameini.GetInt("GAMESETTINGS", "WIDESCREEN", -1);
	perGameSettings_dsiwareBooter = pergameini.GetInt("GAMESETTINGS", "DSIWARE_BOOTER", -1);
	perGameSettings_useBootstrap = pergameini.GetInt("GAMESETTINGS", "USE_BOOTSTRAP", -1);
	perGameSettings_useBootstrapCheat = perGameSettings_useBootstrap;
	perGameSettings_saveRelocation = pergameini.GetInt("GAMESETTINGS", "SAVE_RELOCATION", -1);
}

void savePerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath);
	if (isHomebrew[cursorPosOnScreen]) {
		if (!ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
		if (isModernHomebrew[cursorPosOnScreen]) {
			pergameini.SetInt("GAMESETTINGS", "REGION", perGameSettings_region);
		}
		if (!ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "RAM_DISK", perGameSettings_ramDiskNo);
		pergameini.SetInt("GAMESETTINGS", "DIRECT_BOOT", perGameSettings_directBoot);
		if (isDSiMode() || !ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		}
		if (dsiFeatures()) {
			pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		}
		if (!ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", perGameSettings_bootstrapFile);
			pergameini.SetInt("GAMESETTINGS", "USE_BOOTSTRAP", perGameSettings_useBootstrap);
			perGameSettings_useBootstrapCheat = perGameSettings_useBootstrap;
		}
		if (dsiFeatures() && ms().consoleModel >= 2 && sdFound()) {
			pergameini.SetInt("GAMESETTINGS", "WIDESCREEN", perGameSettings_wideScreen);
		}
	} else {
		if ((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) || (dsiFeatures() && romUnitCode[cursorPosOnScreen] > 0) || isDSiWare[cursorPosOnScreen] || !ms().secondaryDevice) {
			if (sys().dsiWramAccess() && !sys().dsiWramMirrored() && !blacklisted_colorLut) pergameini.SetInt("GAMESETTINGS", "PHAT_COLORS", perGameSettings_dsPhatColors);
			pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
		}
		if (!isDSiWare[cursorPosOnScreen] && ((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) || (dsiFeatures() && romUnitCode[cursorPosOnScreen] > 0) || !ms().secondaryDevice)) {
			pergameini.SetInt("GAMESETTINGS", "REGION", perGameSettings_region);
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		} else if (isDSiWare[cursorPosOnScreen]) {
			pergameini.SetInt("GAMESETTINGS", "REGION", perGameSettings_region);
		}
		pergameini.SetInt("GAMESETTINGS", "SAVE_NUMBER", perGameSettings_saveNo);
		if (!blacklisted_cardReadDma) pergameini.SetInt("GAMESETTINGS", "CARD_READ_DMA", perGameSettings_cardReadDMA);
		if (dsiFeatures()) {
			if (!blacklisted_boostCpu) pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		}
		if (!blacklisted_asyncCardRead) pergameini.SetInt("GAMESETTINGS", "ASYNC_CARD_READ", perGameSettings_asyncCardRead);
		if (ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "USE_BOOTSTRAP", perGameSettings_useBootstrap);
			perGameSettings_useBootstrapCheat = perGameSettings_useBootstrap;
		}
		if ((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) || (dsiFeatures() && romUnitCode[cursorPosOnScreen] > 0) || isDSiWare[cursorPosOnScreen] || !ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", perGameSettings_bootstrapFile);
		}
		if (dsiFeatures() && ms().consoleModel >= 2 && sdFound()) {
			pergameini.SetInt("GAMESETTINGS", "WIDESCREEN", perGameSettings_wideScreen);
		}
		if (isDSiWare[cursorPosOnScreen] && !sys().arm7SCFGLocked() && ms().consoleModel == TWLSettings::EDSiRetail) {
			pergameini.SetInt("GAMESETTINGS", "DSIWARE_BOOTER", perGameSettings_dsiwareBooter);
		}
		pergameini.SetInt("GAMESETTINGS", "SAVE_RELOCATION", perGameSettings_saveRelocation);
	}
	pergameini.SaveIniFile( pergamefilepath );
}

bool checkIfShowAPMsg (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini(pergamefilepath);
	return (pergameini.GetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 0) == 0);
}

void dontShowAPMsgAgain (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini(pergamefilepath);
	pergameini.SetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 1);
	pergameini.SaveIniFile(pergamefilepath);
}

bool checkIfShowRAMLimitMsg (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	return (pergameini.GetInt("GAMESETTINGS", "NO_SHOW_RAM_LIMIT", 0) == 0);
}

void dontShowRAMLimitMsgAgain (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini(pergamefilepath);
	pergameini.SetInt("GAMESETTINGS", "NO_SHOW_RAM_LIMIT_MSG", 1);
	pergameini.SaveIniFile(pergamefilepath);
}

bool checkIfDSiMode (std::string filename) {
	if (ms().secondaryDevice && (!dsiFeatures() || bs().b4dsMode || !(perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap))) {
		return false;
	}

	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", (isModernHomebrew[cursorPosOnScreen] ? true : -1));
	if (perGameSettings_dsiMode == -1) {
		return DEFAULT_DSI_MODE;
	} else {
		return perGameSettings_dsiMode;
	}
}

bool showSetDonorRom(u32 arm7size, u32 SDKVersion, bool dsiBinariesFound) {
	if (requiresDonorRom[cursorPosOnScreen]) return false;

	bool usingB4DS = ((!dsiFeatures() || bs().b4dsMode) && ms().secondaryDevice);
	bool dsiEnhancedMbk = (isDSiMode() && *(u32*)0x02FFE1A0 == 0x00403000 && sys().arm7SCFGLocked());

	return ((SDKVersion > 0x2000000 && SDKVersion < 0x3000000
		&& (arm7size == 0x2619C // SDK2.0
		 || arm7size == 0x262A0
		 || arm7size == 0x26A60
		 || arm7size == 0x27218
		 || arm7size == 0x27224
		 || arm7size == 0x2724C
		 || arm7size == 0x27280))
	|| (usingB4DS && SDKVersion > 0x5000000	// SDK5 (NTR)
	 && (arm7size==0x23708
	  || arm7size==0x2378C
	  || arm7size==0x237F0
	  || arm7size==0x26370
	  || arm7size==0x2642C
	  || arm7size==0x26488
	  || arm7size==0x26BEC
	  || arm7size==0x27618
	  || arm7size==0x2762C
	  || arm7size==0x29CEC))
	|| ((usingB4DS || (dsiEnhancedMbk && dsiBinariesFound)) && SDKVersion > 0x5000000	// SDK5 (TWL)
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
	  || arm7size==0x2C5B4)));
}

bool showSetDonorRomDSiWare(u32 arm7size) {
	if (requiresDonorRom[cursorPosOnScreen] || !isDSiMode() || (*(u32*)0x02FFE1A0 == 0x00403000 && sys().arm7SCFGLocked())) return false;

	return (arm7size==0x1D43C
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
	 || arm7size==0x28E54);
}

bool donorRomTextShown = false;
void revertDonorRomText(void) {
	if (!donorRomTextShown || strncmp(SET_AS_DONOR_ROM, "Done!", 5) != 0) return;

	sprintf(SET_AS_DONOR_ROM, "Set as Donor ROM");
}

const char* getCodenameString(void) {
	if (bnrRomType[cursorPosOnScreen] == 3) {
		return "CGB";
	}
	return bnrRomType[cursorPosOnScreen] == 1 ? "AGB" : (romUnitCode[cursorPosOnScreen] > 0 ? "TWL" : "NTR");
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
	if (ms().macroMode) {
		lcdMainOnBottom();
		lcdSwapped = true;
	}

	int pressed = 0, held = 0;

	keysSetRepeat(25, 5); // Slow down key repeat

	clearText();
	displayDiskIcon(ms().secondaryDevice);

	perGameSettings_cursorPosition = 0;
	loadPerGameSettings(filename);

	std::string filenameForInfo = filename;
	if (extension(filenameForInfo, {".argv"})) {
		std::vector<char*> argarray;

		FILE *argfile = fopen(filenameForInfo.c_str(),"rb");
		char str[PATH_MAX], *pstr;
		const char seps[] = "\n\r\t ";

		while (fgets(str, PATH_MAX, argfile)) {
			// Find comment and end string there
			if ((pstr = strchr(str, '#')) )
				*pstr = '\0';

			// Tokenize arguments
			pstr = strtok(str, seps);

			while (pstr != NULL) {
				argarray.push_back(strdup(pstr));
				pstr = strtok(NULL, seps);
			}
		}
		fclose(argfile);
		filenameForInfo = argarray.at(0);
	}

	FILE *f_nds_file = fopen(filenameForInfo.c_str(), "rb");

	// Check if blacklisted
	blacklisted_colorLut = false;
	if (sys().dsiWramAccess() && !sys().dsiWramMirrored()) {
		for (unsigned int i = 0; i < sizeof(colorLutBlacklist)/sizeof(colorLutBlacklist[0]); i++) {
			if (memcmp(gameTid[cursorPosOnScreen], colorLutBlacklist[i], 3) == 0) {
				// Found match
				blacklisted_colorLut = true;
			}
		}
	}

	blacklisted_boostCpu = false;
	blacklisted_cardReadDma = false;
	blacklisted_asyncCardRead = false;
	if (!ms().ignoreBlacklists) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
			if (memcmp(gameTid[cursorPosOnScreen], twlClockExcludeList[i], 3) == 0) {
				// Found match
				blacklisted_boostCpu = true;
			}
		}

		for (unsigned int i = 0; i < sizeof(cardReadDMAExcludeList)/sizeof(cardReadDMAExcludeList[0]); i++) {
			if (memcmp(gameTid[cursorPosOnScreen], cardReadDMAExcludeList[i], 3) == 0) {
				// Found match
				blacklisted_cardReadDma = true;
			}
		}

		for (unsigned int i = 0; i < sizeof(asyncReadExcludeList)/sizeof(asyncReadExcludeList[0]); i++) {
			if (memcmp(gameTid[cursorPosOnScreen], asyncReadExcludeList[i], 3) == 0) {
				// Found match
				blacklisted_asyncCardRead = true;
			}
		}
	}

	bool showSDKVersion = false;
	u32 SDKVersion = 0;
	u8 sdkSubVer = 0;
	char sdkSubVerChar[8] = {0};
	if (bnrRomType[cursorPosOnScreen] == 0 && (memcmp(gameTid[cursorPosOnScreen], "HND", 3) == 0 || memcmp(gameTid[cursorPosOnScreen], "HNE", 3) == 0 || !isHomebrew[cursorPosOnScreen])) {
		SDKVersion = getSDKVersion(f_nds_file);
		tonccpy(&sdkSubVer, (u8*)&SDKVersion+2, 1);
		sprintf(sdkSubVerChar, "%d", sdkSubVer);
		showSDKVersion = true;
	}
	u32 arm9off = 0;
	u32 arm9size = 0;
	u32 arm7off = 0;
	u32 arm7size = 0;
	u32 ovlOff = 0;
	u32 ovlSize = 0;
	u32 romSize = 0;
	u32 pubSize = 0;
	u32 prvSize = 0;
	bool usesCloneboot = false;
	bool dsiBinariesFound = false;
	if (bnrRomType[cursorPosOnScreen] == 0) {
		fseek(f_nds_file, 0x20, SEEK_SET);
		fread(&arm9off, sizeof(u32), 1, f_nds_file);
		fseek(f_nds_file, 0x2C, SEEK_SET);
		fread(&arm9size, sizeof(u32), 1, f_nds_file);
		fseek(f_nds_file, 0x30, SEEK_SET);
		fread(&arm7off, sizeof(u32), 1, f_nds_file);
		fseek(f_nds_file, 0x3C, SEEK_SET);
		fread(&arm7size, sizeof(u32), 1, f_nds_file);
		fseek(f_nds_file, 0x50, SEEK_SET);
		fread(&ovlOff, sizeof(u32), 1, f_nds_file);
		fread(&ovlSize, sizeof(u32), 1, f_nds_file);
		fseek(f_nds_file, 0x80, SEEK_SET);
		fread(&romSize, sizeof(u32), 1, f_nds_file);
		fseek(f_nds_file, 0x238, SEEK_SET);
		fread(&pubSize, sizeof(u32), 1, f_nds_file);
		fread(&prvSize, sizeof(u32), 1, f_nds_file);
		fseek(f_nds_file, romSize, SEEK_SET);
		u32 clonebootFlag = 0;
		fread(&clonebootFlag, sizeof(u32), 1, f_nds_file);
		usesCloneboot = (clonebootFlag == 0x16361);
		dsiBinariesFound = checkDsiBinaries(f_nds_file);
	}
	fclose(f_nds_file);

	const bool largeArm9 = (arm9size >= 0x380000 && isModernHomebrew[cursorPosOnScreen]);

	if (romSize > 0) {
		if (usesCloneboot) {
			romSize -= 0x4000;	// First 16KB
			romSize += 0x88;	// RSA key
		} else if (ovlOff == 0 || ovlSize == 0) {
			romSize -= (arm7off + arm7size);
		} else if (ovlOff > arm7off) {
			romSize -= (arm9off + arm9size);
		} else {
			romSize -= ovlOff;
		}
	}

	#define sharedWramEnabled (!sys().arm7SCFGLocked() || *(u32*)0x02FFE1A0 != 0x00403000)
	u32 dsiWramAmount = (sharedWramEnabled ? 0x88000 : 0x80000);
	if (colorTable && sys().dsiWramAccess() && !sys().dsiWramMirrored()) {
		dsiWramAmount -= 0x80000;
		dsiWramAmount += 0x32800;
	}
	u32 romSizeLimit = (ms().consoleModel > 0 ? 0x1BE0000 : 0xBE0000) + ((sys().dsiWramAccess() && !sys().dsiWramMirrored()) ? dsiWramAmount : (sharedWramEnabled ? 0x8000 : 0));
	romSizeLimit -= 0x400000; // Account for DSi mode setting
	const u32 romSizeLimitTwl = (ms().consoleModel > 0 ? 0x1000000 : 0);
	const bool romLoadableInRam = (romSize <= (romUnitCode[cursorPosOnScreen] > 0 ? romSizeLimitTwl : romSizeLimit));

	extern bool dsiWareCompatibleB4DS(void);
	bool showPerGameSettings = (bnrRomType[cursorPosOnScreen] == 0 && !isDSiWare[cursorPosOnScreen]);
	if (bnrRomType[cursorPosOnScreen] == 0 && (dsiFeatures() || dsiWareCompatibleB4DS() || !ms().secondaryDevice) && !isHomebrew[cursorPosOnScreen] && isDSiWare[cursorPosOnScreen]) {
		showPerGameSettings = true;
	}
	/*if (!(perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) && !isHomebrew[cursorPosOnScreen] && REG_SCFG_EXT == 0) {
		showPerGameSettings = false;
	}*/
	bool runInShown = false;

	bool showCheats = (((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap)
	|| !ms().kernelUseable
	|| !ms().secondaryDevice) && bnrRomType[cursorPosOnScreen] == 0 && !isHomebrew[cursorPosOnScreen] && !isDSiWare[cursorPosOnScreen]
	&& memcmp(gameTid[cursorPosOnScreen], "HND", 3) != 0
	&& memcmp(gameTid[cursorPosOnScreen], "HNE", 3) != 0);

	firstPerGameOpShown = 0;
	perGameOps = -1;
	for (int i = 0; i < 17; i++) {
		perGameOp[i] = -1;
	}
	if (isHomebrew[cursorPosOnScreen]) {		// Per-game settings for homebrew
		perGameOps++;
		perGameOp[perGameOps] = 0;	// Language
		if (isModernHomebrew[cursorPosOnScreen]) {
			perGameOps++;
			perGameOp[perGameOps] = 11;	// Region
		}
		if (!largeArm9) {
			if (!ms().secondaryDevice) {
				perGameOps++;
				perGameOp[perGameOps] = 1;	// RAM disk number
				if (!sys().arm7SCFGLocked() && ms().consoleModel == TWLSettings::EDSiRetail) {
					perGameOps++;
					perGameOp[perGameOps] = 14;	// Game Loader
				}
			} else {
				perGameOps++;
				perGameOp[perGameOps] = 6;	// Direct boot
			}
			if (isDSiMode() || !ms().secondaryDevice) {
				perGameOps++;
				perGameOp[perGameOps] = 2;	// Run in
				runInShown = true;
			}
			if (dsiFeatures() || !ms().secondaryDevice) {
				perGameOps++;
				perGameOp[perGameOps] = 3;	// ARM9 CPU Speed
				perGameOps++;
				perGameOp[perGameOps] = 4;	// VRAM Boost
			}
			if (!ms().secondaryDevice) {
				perGameOps++;
				perGameOp[perGameOps] = 7;	// Bootstrap
			}
		}
		if (dsiFeatures() && ms().consoleModel >= 2 && sdFound() && (gameTid[cursorPosOnScreen][0] == 'W' || romVersion[cursorPosOnScreen] == 0x57)) {
			perGameOps++;
			perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
		}
	} else if (showPerGameSettings && isDSiWare[cursorPosOnScreen]) {	// Per-game settings for DSiWare
		if ((perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) || sys().arm7SCFGLocked() || ms().consoleModel > 0) {
			perGameOps++;
			perGameOp[perGameOps] = 0;	// Language
			perGameOps++;
			perGameOp[perGameOps] = 11;	// Region
		}
		if ((ms().saveLocation != TWLSettings::EGamesFolder) && (pubSize > 0 || prvSize > 0)) {
			perGameOps++;
			perGameOp[perGameOps] = 1;	// Save number
		}
		if (!sys().arm7SCFGLocked() && ms().consoleModel == TWLSettings::EDSiRetail) {
			perGameOps++;
			perGameOp[perGameOps] = 13;	// DSiWare booter
		}
		if ((perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) || !dsiFeatures() || (ms().secondaryDevice && bs().b4dsMode) || !ms().dsiWareToSD || sys().arm7SCFGLocked() || ms().consoleModel > 0) {
			if ((!ms().secondaryDevice || (dsiFeatures() && !bs().b4dsMode)) && !sys().scfgSdmmcEnabled() && !blacklisted_cardReadDma) {
				perGameOps++;
				perGameOp[perGameOps] = 5;	// Card Read DMA
			}
			if ((!ms().secondaryDevice || (dsiFeatures() && !bs().b4dsMode)) && !sys().scfgSdmmcEnabled() && !romLoadableInRam && !blacklisted_asyncCardRead) {
				perGameOps++;
				perGameOp[perGameOps] = 12;	// Async Card Read
			}
			if (((dsiFeatures() && !bs().b4dsMode) || !ms().secondaryDevice) && sys().dsiWramAccess() && !sys().dsiWramMirrored() && !blacklisted_colorLut) {
				perGameOps++;
				perGameOp[perGameOps] = 16;	// DS Phat Colors
			}
			perGameOps++;
			perGameOp[perGameOps] = 7;	// Bootstrap
			if (((dsiFeatures() && sdFound()) || !ms().secondaryDevice) && ms().consoleModel >= 2 && (!isDSiMode() || !sys().arm7SCFGLocked()) && widescreenFound) {
				perGameOps++;
				perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
			}
			showCheats = true;
		}
		if (a7mbk6[cursorPosOnScreen] == 0x080037C0 ? showSetDonorRomDSiWare(arm7size) : showSetDonorRom(arm7size, SDKVersion, dsiBinariesFound)) {
			perGameOps++;
			perGameOp[perGameOps] = 9;	// Set as Donor ROM
			donorRomTextShown = true;
		} else {
			donorRomTextShown = false;
		}
	} else if (showPerGameSettings) {	// Per-game settings for retail/commercial games
		const bool bootstrapEnabled = ((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) || (dsiFeatures() && romUnitCode[cursorPosOnScreen] > 0) || (ms().secondaryDevice && (!ms().kernelUseable || romUnitCode[cursorPosOnScreen] == 3)) || !ms().secondaryDevice);
		if (bootstrapEnabled) {
			perGameOps++;
			perGameOp[perGameOps] = 0;	// Language
		}
		if (bootstrapEnabled && romUnitCode[cursorPosOnScreen] == 3) {
			perGameOps++;
			perGameOp[perGameOps] = 11;	// Region
		}
		if (ms().saveLocation != TWLSettings::EGamesFolder) {
			perGameOps++;
			perGameOp[perGameOps] = 1;	// Save number
		}
		if (((dsiFeatures() && (((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) && isDSiMode()) || romUnitCode[cursorPosOnScreen] > 0) && !bs().b4dsMode) || !ms().secondaryDevice) && !blacklisted_boostCpu) {
			perGameOps++;
			perGameOp[perGameOps] = 2;	// Run in
			runInShown = true;
		}
		if ((dsiFeatures() || !ms().secondaryDevice) && romUnitCode[cursorPosOnScreen] < 3) {
			if (!blacklisted_boostCpu) {
				perGameOps++;
				perGameOp[perGameOps] = 3;	// ARM9 CPU Speed
			}
			perGameOps++;
			perGameOp[perGameOps] = 4;	// VRAM Boost
		}
		if (bootstrapEnabled && (!ms().secondaryDevice || (dsiFeatures() && !bs().b4dsMode)) && !blacklisted_cardReadDma) {
			perGameOps++;
			perGameOp[perGameOps] = 5;	// Card Read DMA
		}
		if (ms().secondaryDevice && ms().kernelUseable && romUnitCode[cursorPosOnScreen] < 3) {
			perGameOps++;
			perGameOp[perGameOps] = 14;	// Game Loader
		}
		if (bootstrapEnabled) {
			if ((!ms().secondaryDevice || (dsiFeatures() && !bs().b4dsMode)) && !romLoadableInRam && !blacklisted_asyncCardRead) {
				perGameOps++;
				perGameOp[perGameOps] = 12;	// Async Card Read
			}
			if (((dsiFeatures() && !bs().b4dsMode) || !ms().secondaryDevice) && sys().dsiWramAccess() && !sys().dsiWramMirrored() && !blacklisted_colorLut) {
				perGameOps++;
				perGameOp[perGameOps] = 16;	// DS Phat Colors
			}
			if ((!ms().secondaryDevice && !sys().arm7SCFGLocked()) || (sys().isRegularDS() && (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA))) {
				perGameOps++;
				perGameOp[perGameOps] = 17;	// Save Relocation
			}
			perGameOps++;
			perGameOp[perGameOps] = 7;	// Bootstrap
			if (((dsiFeatures() && sdFound()) || !ms().secondaryDevice) && widescreenFound) {
				perGameOps++;
				perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
			}
			if (a7mbk6[cursorPosOnScreen] == 0x080037C0 ? showSetDonorRomDSiWare(arm7size) : showSetDonorRom(arm7size, SDKVersion, dsiBinariesFound)) {
				perGameOps++;
				perGameOp[perGameOps] = 9;	// Set as Donor ROM
				donorRomTextShown = true;
			} else {
				donorRomTextShown = false;
			}
		} else if (!dsiFeatures()) {
			if (a7mbk6[cursorPosOnScreen] != 0x080037C0 && showSetDonorRom(arm7size, SDKVersion, dsiBinariesFound)) {
				perGameOps++;
				perGameOp[perGameOps] = 9;	// Set as Donor ROM
				donorRomTextShown = true;
			} else {
				donorRomTextShown = false;
			}
		}
	}

	bool savExists[10] = {false};
	if (isHomebrew[cursorPosOnScreen] && !largeArm9) {
		snprintf (gameTIDText, sizeof(gameTIDText), gameTid[cursorPosOnScreen][0]==0 ? "" : "TID: %s", gameTid[cursorPosOnScreen]);

		for (int i = 0; i < 10; i++) {
			std::string path("ramdisks/" + filenameForInfo.substr(0, filenameForInfo.find_last_of('.')) + getImgExtension(i));
			if (i > 0)
				path += std::to_string(i);
			savExists[i] = access(path.c_str(), F_OK) == 0;
		}
	} else if (!isHomebrew[cursorPosOnScreen]) {
		snprintf (gameTIDText, sizeof(gameTIDText), gameTid[cursorPosOnScreen][0]==0 ? "" : "%s-%s-%s", getCodenameString(), gameTid[cursorPosOnScreen], getRegionString(gameTid[cursorPosOnScreen][3]));

		if (showPerGameSettings) {
			int saveNoBak = perGameSettings_saveNo;
			for (int i = 0; i < 10; i++) {
				perGameSettings_saveNo = i;
				if (dsiFeatures() && (!bs().b4dsMode || !ms().secondaryDevice) && isDSiWare[cursorPosOnScreen] && (pubSize > 0 || prvSize > 0)) {
					std::string path("saves/" + filenameForInfo.substr(0, filenameForInfo.find_last_of('.')));
					savExists[i] = access((path + getPubExtension()).c_str(), F_OK) == 0 || access((path + getPrvExtension()).c_str(), F_OK) == 0;
				} else {
					std::string path("saves/" + filenameForInfo.substr(0, filenameForInfo.find_last_of('.')) + getSavExtension());
					savExists[i] = access(path.c_str(), F_OK) == 0;
				}
			}
			perGameSettings_saveNo = saveNoBak;
		}
	}
	displayDiskIcon(false);

	char saveNoDisplay[16];

	if (bnrRomType[cursorPosOnScreen] == 0) {
		if ((SDKVersion > 0x1000000) && (SDKVersion < 0x2000000)) {
			SDKnumbertext = ("SDK ver: 1."+(std::string)sdkSubVerChar);
		} else if ((SDKVersion > 0x2000000) && (SDKVersion < 0x3000000)) {
			SDKnumbertext = ("SDK ver: 2."+(std::string)sdkSubVerChar);
		} else if ((SDKVersion > 0x3000000) && (SDKVersion < 0x4000000)) {
			SDKnumbertext = ("SDK ver: 3."+(std::string)sdkSubVerChar);
		} else if ((SDKVersion > 0x4000000) && (SDKVersion < 0x5000000)) {
			SDKnumbertext = ("SDK ver: 4."+(std::string)sdkSubVerChar);
		} else if ((SDKVersion > 0x5000000) && (SDKVersion < 0x6000000)) {
			SDKnumbertext = ("SDK ver: 5."+(std::string)sdkSubVerChar);
		} else {
			SDKnumbertext = "SDK ver: ???";
		}
	}
	if (!showPerGameSettings) {
		dialogboxHeight = 0;
	} else {
		dialogboxHeight = 4;
	}

	sprintf(SET_AS_DONOR_ROM, "Set as Donor ROM");

	showdialogbox = true;

	while (1) {
		clearText(false);

		printSmall(false, 0, 74, showPerGameSettings ? "Game settings" : "Info", Alignment::center, FontPalette::formTitleText);
		if (showSDKVersion) printSmall(false, 24, 90, SDKnumbertext, Alignment::left, FontPalette::formText);
		printSmall(false, 232, 90, gameTIDText, Alignment::right, FontPalette::formText);

		int perGameOpXpos = 24;
		int perGameOpYpos = 104;
		bool flashcardKernelOnly = (!(perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) && ms().secondaryDevice && !isHomebrew[cursorPosOnScreen] && romUnitCode[cursorPosOnScreen] == 2 && (perGameSettings_dsiMode==-1 ? !DEFAULT_DSI_MODE : perGameSettings_dsiMode==0));

		for (int i = firstPerGameOpShown; i < firstPerGameOpShown+4; i++) {
		if (!showPerGameSettings || perGameOp[i] == -1) break;
		const FontPalette highlighted = (i == perGameSettings_cursorPosition) ? FontPalette::user : FontPalette::formText;
		switch (perGameOp[i]) {
			case 0:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Language:", Alignment::left, highlighted);
				if (perGameSettings_language == -1 || flashcardKernelOnly) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "System", Alignment::right, highlighted);
				} else if (perGameSettings_language == -2) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_language == 0) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Japanese", Alignment::right, highlighted);
				} else if (perGameSettings_language == 1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "English", Alignment::right, highlighted);
				} else if (perGameSettings_language == 2) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "French", Alignment::right, highlighted);
				} else if (perGameSettings_language == 3) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "German", Alignment::right, highlighted);
				} else if (perGameSettings_language == 4) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Italian", Alignment::right, highlighted);
				} else if (perGameSettings_language == 5) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Spanish", Alignment::right, highlighted);
				} else if (perGameSettings_language == 6) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Chinese", Alignment::right, highlighted);
				} else if (perGameSettings_language == 7) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Korean", Alignment::right, highlighted);
				}
				break;
			case 1:
				if (isHomebrew[cursorPosOnScreen]) {
					printSmall(false, perGameOpXpos, perGameOpYpos, "RAM disk:", Alignment::left, highlighted);
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "%i%s", perGameSettings_ramDiskNo, savExists[perGameSettings_ramDiskNo] ? "*" : "");
				} else {
					printSmall(false, perGameOpXpos, perGameOpYpos, "Save Number:", Alignment::left, highlighted);
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "%i%s", perGameSettings_saveNo, savExists[perGameSettings_saveNo] ? "*" : "");
				}
				if (isHomebrew[cursorPosOnScreen] && perGameSettings_ramDiskNo == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "None", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, saveNoDisplay, Alignment::right, highlighted);
				}
				break;
			case 2:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Run in:", Alignment::left, highlighted);
				if (perGameSettings_dsiMode == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_dsiMode > 0) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "DSi mode", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "DS mode", Alignment::right, highlighted);
				}
				break;
			case 3:
				printSmall(false, perGameOpXpos, perGameOpYpos, "ARM9 CPU Speed:", Alignment::left, highlighted);
				if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE && romUnitCode[cursorPosOnScreen] > 0) : perGameSettings_dsiMode > 0) && runInShown) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "133mhz (TWL)", Alignment::right, highlighted);
				} else {
					if (perGameSettings_boostCpu == -1) {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
					} else if (perGameSettings_boostCpu == 1) {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, "133mhz (TWL)", Alignment::right, highlighted);
					} else {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, "67mhz (NTR)", Alignment::right, highlighted);
					}
				}
				break;
			case 4:
				printSmall(false, perGameOpXpos, perGameOpYpos, "VRAM Mode:", Alignment::left, highlighted);
				if (((isHomebrew[cursorPosOnScreen] && isModernHomebrew[cursorPosOnScreen]) || romUnitCode[cursorPosOnScreen] > 0) && (perGameSettings_dsiMode==-1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode > 0) && runInShown) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, (isHomebrew[cursorPosOnScreen] && isModernHomebrew[cursorPosOnScreen]) ? "DSi mode" : "Auto", Alignment::right, highlighted);
				} else {
					if (perGameSettings_boostVram == -1) {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
					} else if (perGameSettings_boostVram == 1) {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, "DSi mode", Alignment::right, highlighted);
					} else {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, "DS mode", Alignment::right, highlighted);
					}
				}
				break;
			case 5:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Card Read DMA:", Alignment::left, highlighted);
				if (perGameSettings_cardReadDMA == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_cardReadDMA == 1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "On", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Off", Alignment::right, highlighted);
				}
				break;
			case 6:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Direct Boot:", Alignment::left, highlighted);
				if (perGameSettings_directBoot) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Yes", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "No", Alignment::right, highlighted);
				}
				break;
			case 7:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Bootstrap:", Alignment::left, highlighted);
				if (flashcardKernelOnly) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Not Used", Alignment::right, highlighted);
				} else if (perGameSettings_bootstrapFile == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_bootstrapFile == 1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Nightly", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Release", Alignment::right, highlighted);
				}
				break;
			case 8:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Screen Aspect Ratio:", Alignment::left, highlighted);
				if (flashcardKernelOnly) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Not Used", Alignment::right, highlighted);
				} else if (perGameSettings_wideScreen == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_wideScreen == 1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "16:10", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "4:3", Alignment::right, highlighted);
				}
				break;
			case 9:
				printSmall(false, 0, perGameOpYpos, SET_AS_DONOR_ROM, Alignment::center, highlighted);
				break;
			case 11:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Region:", Alignment::left, highlighted);
				if (perGameSettings_region == -2) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_region == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "System", Alignment::right, highlighted);
				} else if (perGameSettings_region == 0) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Japan", Alignment::right, highlighted);
				} else if (perGameSettings_region == 1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "USA", Alignment::right, highlighted);
				} else if (perGameSettings_region == 2) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Europe", Alignment::right, highlighted);
				} else if (perGameSettings_region == 3) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Australia", Alignment::right, highlighted);
				} else if (perGameSettings_region == 4) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "China", Alignment::right, highlighted);
				} else if (perGameSettings_region == 5) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Korea", Alignment::right, highlighted);
				}
				break;
			case 12:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Asynch Card Read:", Alignment::left, highlighted);
				if (perGameSettings_asyncCardRead == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_asyncCardRead == 1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "On", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Off", Alignment::right, highlighted);
				}
				break;
			case 13:
				printSmall(false, perGameOpXpos, perGameOpYpos, "DSiWare Booter:", Alignment::left, highlighted);
				if (perGameSettings_dsiwareBooter == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_dsiwareBooter == 1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "nds-bootstrap", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Unlaunch", Alignment::right, highlighted);
				}
				break;
			case 14:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Game Loader:", Alignment::left, highlighted);
				if (perGameSettings_useBootstrap == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (isHomebrew[cursorPosOnScreen]) {
					if (perGameSettings_useBootstrap == 1) {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, isModernHomebrew[cursorPosOnScreen] ? "Direct" : "nds-bootstrap", Alignment::right, highlighted);
					} else {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Unlaunch", Alignment::right, highlighted);
					}
				} else {
					if (perGameSettings_useBootstrap == 1) {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, "nds-bootstrap", Alignment::right, highlighted);
					} else {
						printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Kernel", Alignment::right, highlighted);
					}
				}
				break;
			case 16:
				printSmall(false, perGameOpXpos, perGameOpYpos, "DS Phat Colors:", Alignment::left, highlighted);
				if (perGameSettings_dsPhatColors == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_dsPhatColors == 1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "On", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Off", Alignment::right, highlighted);
				}
				break;
			case 17:
				printSmall(false, perGameOpXpos, perGameOpYpos, "Save Relocation:", Alignment::left, highlighted);
				if (perGameSettings_saveRelocation == -1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Default", Alignment::right, highlighted);
				} else if (perGameSettings_saveRelocation == 1) {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, (ms().showMicroSd || ms().secondaryDevice) ? "microSD Card" : "SD Card", Alignment::right, highlighted);
				} else {
					printSmall(false, 256-perGameOpXpos, perGameOpYpos, "Game Card", Alignment::right, highlighted);
				}
				break;
		}
		perGameOpYpos += 12;
		}
		if (!showPerGameSettings && !showCheats) {
			printSmall(false, 0, perGameOpYpos+5, " OK", Alignment::center, FontPalette::formText);
		} else {
			printSmall(false, 0, perGameOpYpos+5, showCheats ? " Cheats   Back" : " Back", Alignment::center, FontPalette::formText);
		}
		updateText(false);
		do {
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
			bgOperations(true);
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
						if (isHomebrew[cursorPosOnScreen]) {
							perGameSettings_ramDiskNo--;
							if (perGameSettings_ramDiskNo < -1) perGameSettings_ramDiskNo = 9;
						} else {
							perGameSettings_saveNo--;
							if (perGameSettings_saveNo < 0) perGameSettings_saveNo = 9;
						}
						break;
					case 2:
						perGameSettings_dsiMode--;
						if (!isHomebrew[cursorPosOnScreen]) {
							if ((perGameSettings_dsiMode == 1 && romUnitCode[cursorPosOnScreen] == 0)
							 || (perGameSettings_dsiMode == 2 && romUnitCode[cursorPosOnScreen] > 0)) perGameSettings_dsiMode--;
						}
						if (perGameSettings_dsiMode < -1) perGameSettings_dsiMode = 2-isHomebrew[cursorPosOnScreen];
						if (!isHomebrew[cursorPosOnScreen]) {
							if (perGameSettings_dsiMode == 2 && romUnitCode[cursorPosOnScreen] > 0) perGameSettings_dsiMode--;
						}
						break;
					case 3:
						if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE == 0 || romUnitCode[cursorPosOnScreen] == 0) : perGameSettings_dsiMode < 1) || !runInShown) {
							perGameSettings_boostCpu--;
							if (perGameSettings_boostCpu < -1) perGameSettings_boostCpu = 1;
						}
						break;
					case 4:
						if (((!isHomebrew[cursorPosOnScreen] || !isModernHomebrew[cursorPosOnScreen]) && romUnitCode[cursorPosOnScreen] == 0) || (perGameSettings_dsiMode==-1 ? DEFAULT_DSI_MODE == TWLSettings::EDSMode : perGameSettings_dsiMode < 1) || !runInShown) {
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
						perGameSettings_dsiwareBooter--;
						if (perGameSettings_dsiwareBooter < -1) perGameSettings_dsiwareBooter = 1;
						break;
					case 14:
						perGameSettings_useBootstrap--;
						if (perGameSettings_useBootstrap < -1) perGameSettings_useBootstrap = 1;
						break;
					case 16:
						perGameSettings_dsPhatColors--;
						if (perGameSettings_dsPhatColors < -1) perGameSettings_dsPhatColors = 1;
						break;
					case 17:
						perGameSettings_saveRelocation++;
						if (perGameSettings_saveRelocation > 1) perGameSettings_saveRelocation = -1;
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
						if (isHomebrew[cursorPosOnScreen]) {
							perGameSettings_ramDiskNo++;
							if (perGameSettings_ramDiskNo > 9) perGameSettings_ramDiskNo = -1;
						} else {
							perGameSettings_saveNo++;
							if (perGameSettings_saveNo > 9) perGameSettings_saveNo = 0;
						}
						break;
					case 2:
						perGameSettings_dsiMode++;
						if (!isHomebrew[cursorPosOnScreen]) {
							if ((perGameSettings_dsiMode == 1 && romUnitCode[cursorPosOnScreen] == 0)
							 || (perGameSettings_dsiMode == 2 && romUnitCode[cursorPosOnScreen] > 0)) perGameSettings_dsiMode++;
						}
						if (perGameSettings_dsiMode > 2-isHomebrew[cursorPosOnScreen]) perGameSettings_dsiMode = -1;
						break;
					case 3:
						if ((perGameSettings_dsiMode==-1 ? (DEFAULT_DSI_MODE == 0 || romUnitCode[cursorPosOnScreen] == 0) : perGameSettings_dsiMode < 1) || !runInShown) {
							perGameSettings_boostCpu++;
							if (perGameSettings_boostCpu > 1) perGameSettings_boostCpu = -1;
						}
						break;
					case 4:
						if (((!isHomebrew[cursorPosOnScreen] || !isModernHomebrew[cursorPosOnScreen]) && romUnitCode[cursorPosOnScreen] == 0) || (perGameSettings_dsiMode==-1 ? DEFAULT_DSI_MODE == TWLSettings::EDSMode : perGameSettings_dsiMode < 1) || !runInShown) {
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
						const char* pathDefine = a7mbk6[cursorPosOnScreen] == 0x080037C0 ? "DONORTWLONLY_NDS_PATH" : "DONORTWL_NDS_PATH"; // SDK5.x
						if (arm7size == 0x26CC8
						 || arm7size == 0x28E54
						 || arm7size == 0x29EE8) {
							pathDefine = a7mbk6[cursorPosOnScreen] == 0x080037C0 ? "DONORTWLONLY0_NDS_PATH" : "DONORTWL0_NDS_PATH"; // SDK5.0
						} else if (SDKVersion > 0x5000000
								 && (arm7size==0x23708
								  || arm7size==0x2378C
								  || arm7size==0x237F0
								  || arm7size==0x26370
								  || arm7size==0x2642C
								  || arm7size==0x26488
								  || arm7size==0x27618
								  || arm7size==0x2762C
								  || arm7size==0x29CEC)) {
							pathDefine = "DONOR5_NDS_PATH"; // SDK5.x (NTR)
						} else if (SDKVersion > 0x5000000
								 && (arm7size==0x26BEC)) {
							pathDefine = "DONOR5_NDS_PATH_ALT"; // SDK5.x (NTR)
						} else if (SDKVersion > 0x2000000 && SDKVersion < 0x3000000
								&& (arm7size == 0x2619C
								 || arm7size == 0x262A0
								 || arm7size == 0x26A60
								 || arm7size == 0x27218
								 || arm7size == 0x27224
								 || arm7size == 0x2724C
								 || arm7size == 0x27280)) {
							pathDefine = "DONOR20_NDS_PATH"; // SDK2.0
						}
						std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						const char *bootstrapinipath = sys().isRunFromSD() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC;
						CIniFile bootstrapini(bootstrapinipath);
						bootstrapini.SetString("NDS-BOOTSTRAP", pathDefine, romFolderNoSlash+"/"+filename);
						displayDiskIcon(!sys().isRunFromSD());
						bootstrapini.SaveIniFile(bootstrapinipath);
						displayDiskIcon(false);
						sprintf(SET_AS_DONOR_ROM, "Done!");
					  }
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
						perGameSettings_dsiwareBooter++;
						if (perGameSettings_dsiwareBooter > 1) perGameSettings_dsiwareBooter = -1;
						break;
					case 14:
						perGameSettings_useBootstrap++;
						if (perGameSettings_useBootstrap > 1) perGameSettings_useBootstrap = -1;
						break;
					case 16:
						perGameSettings_dsPhatColors++;
						if (perGameSettings_dsPhatColors > 1) perGameSettings_dsPhatColors = -1;
						break;
					case 17:
						perGameSettings_saveRelocation--;
						if (perGameSettings_saveRelocation < -1) perGameSettings_saveRelocation = 1;
						break;
				}
				perGameSettingsChanged = true;
			}

			if (pressed & KEY_B) {
				if (perGameSettingsChanged) {
					displayDiskIcon(ms().secondaryDevice);
					savePerGameSettings(filename);
					displayDiskIcon(false);
					perGameSettingsChanged = false;
				}
				break;
			}
		}
		if ((pressed & KEY_X) && !isHomebrew[cursorPosOnScreen] && showCheats) {
			CheatCodelist codelist;
			codelist.selectCheats(filename);
		}
	}
	clearText(false);
	showdialogbox = false;
	dialogboxHeight = 0;
	updateText(false);
	keysSetRepeat(10, 2); // Reset key repeat

	if (ms().macroMode) {
		lcdMainOnTop();
		lcdSwapped = false;
	}
}

std::string getSavExtension(void) {
	if (ms().saveLocation == TWLSettings::EGamesFolder || perGameSettings_saveNo == 0) {
		return ".sav";
	} else {
		return ".sav" + std::to_string(perGameSettings_saveNo);
	}
}

std::string getPubExtension(void) {
	if (ms().saveLocation == TWLSettings::EGamesFolder || perGameSettings_saveNo == 0) {
		return ".pub";
	} else {
		return ".pu" + std::to_string(perGameSettings_saveNo);
	}
}

std::string getPrvExtension(void) {
	if (ms().saveLocation == TWLSettings::EGamesFolder || perGameSettings_saveNo == 0) {
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
