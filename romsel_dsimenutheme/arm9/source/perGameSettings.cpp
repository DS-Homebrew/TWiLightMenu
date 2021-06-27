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
#include <maxmod9.h>
#include <gl2d.h>

#include "date.h"

#include "ndsheaderbanner.h"
#include "iconTitle.h"
#include "graphics/fontHandler.h"
#include "graphics/graphics.h"
#include "graphics/FontGraphic.h"
#include "graphics/ThemeTextures.h"
#include "language.h"
#include "sound.h"
#include "SwitchState.h"
#include "cheat.h"
#include "errorScreen.h"

#include "gbaswitch.h"
#include "nds_loader_arm9.h"

#include "common/inifile.h"
#include "common/flashcard.h"
#include "common/dsimenusettings.h"
#include "common/systemdetails.h"
#include "myDSiMode.h"

#define SCREEN_COLS 32
#define ENTRIES_PER_SCREEN 15
#define ENTRIES_START_ROW 3
#define ENTRY_PAGE_LENGTH 10


extern bool useTwlCfg;

extern const char *bootstrapinipath;

extern int currentBg;
extern bool displayGameIcons;

std::string SDKnumbertext;

extern bool fadeType;
extern bool showdialogbox;
extern bool dbox_showIcon;

bool perGameSettingsChanged = false;

int perGameSettings_cursorPosition = 0;
bool perGameSettings_directBoot = false;	// Homebrew only
int perGameSettings_dsiMode = -1;
int perGameSettings_language = -2;
int perGameSettings_region = -3;
int perGameSettings_saveNo = 0;
int perGameSettings_ramDiskNo = -1;
int perGameSettings_boostCpu = -1;
int perGameSettings_boostVram = -1;
int perGameSettings_cardReadDMA = -1;
int perGameSettings_asyncCardRead = -1;
int perGameSettings_bootstrapFile = -1;
int perGameSettings_wideScreen = -1;
int perGameSettings_expandRomSpace = -1;

std::string setAsDonorRom = "";

extern int file_count;

char pergamefilepath[256];

extern void RemoveTrailingSlashes(std::string &path);

extern std::string dirContName;

extern mm_sound_effect snd_launch;
extern mm_sound_effect snd_select;
extern mm_sound_effect snd_stop;
extern mm_sound_effect snd_wrong;
extern mm_sound_effect snd_back;
extern mm_sound_effect snd_switch;

extern char usernameRendered[11];
extern bool usernameRenderedDone;

char fileCounter[8];
char gameTIDText[16];
char saveNoDisplay[8];

int firstPerGameOpShown = 0;
int perGameOps = -1;
int perGameOp[10] = {-1};

void loadPerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_directBoot = pergameini.GetInt("GAMESETTINGS", "DIRECT_BOOT", (isModernHomebrew[CURPOS] || ms().secondaryDevice));	// Homebrew only
	if (isHomebrew[CURPOS]) {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", (isModernHomebrew[CURPOS] ? true : false));
	} else {
		perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", -1);
	}
	perGameSettings_language = pergameini.GetInt("GAMESETTINGS", "LANGUAGE", -2);
	perGameSettings_region = pergameini.GetInt("GAMESETTINGS", "REGION", -3);
	if (!dsiFeatures() && (perGameSettings_region == -2 || perGameSettings_region == -1)) perGameSettings_region = -3;
	perGameSettings_saveNo = pergameini.GetInt("GAMESETTINGS", "SAVE_NUMBER", 0);
	perGameSettings_ramDiskNo = pergameini.GetInt("GAMESETTINGS", "RAM_DISK", -1);
	perGameSettings_boostCpu = pergameini.GetInt("GAMESETTINGS", "BOOST_CPU", -1);
	perGameSettings_boostVram = pergameini.GetInt("GAMESETTINGS", "BOOST_VRAM", -1);
	perGameSettings_cardReadDMA = pergameini.GetInt("GAMESETTINGS", "CARD_READ_DMA", -1);
	perGameSettings_asyncCardRead = pergameini.GetInt("GAMESETTINGS", "ASYNC_CARD_READ", -1);
    perGameSettings_bootstrapFile = pergameini.GetInt("GAMESETTINGS", "BOOTSTRAP_FILE", -1);
    perGameSettings_wideScreen = pergameini.GetInt("GAMESETTINGS", "WIDESCREEN", -1);
    perGameSettings_expandRomSpace = pergameini.GetInt("GAMESETTINGS", "EXTENDED_MEMORY", -1);
}

void savePerGameSettings (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (isHomebrew[CURPOS]) {
		pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
		if (isModernHomebrew[CURPOS]) {
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
	} else {
		if (ms().useBootstrap || !ms().secondaryDevice) pergameini.SetInt("GAMESETTINGS", "LANGUAGE", perGameSettings_language);
		if ((dsiFeatures() && ms().useBootstrap) || !ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "REGION", perGameSettings_region);
			pergameini.SetInt("GAMESETTINGS", "DSI_MODE", perGameSettings_dsiMode);
		}
		pergameini.SetInt("GAMESETTINGS", "SAVE_NUMBER", perGameSettings_saveNo);
		if (dsiFeatures()) {
			pergameini.SetInt("GAMESETTINGS", "BOOST_CPU", perGameSettings_boostCpu);
			pergameini.SetInt("GAMESETTINGS", "BOOST_VRAM", perGameSettings_boostVram);
		}
		if (!ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "CARD_READ_DMA", perGameSettings_cardReadDMA);
			pergameini.SetInt("GAMESETTINGS", "ASYNC_CARD_READ", perGameSettings_asyncCardRead);
		}
		if (ms().useBootstrap || !ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "BOOTSTRAP_FILE", perGameSettings_bootstrapFile);
		}
		if (dsiFeatures() && ms().consoleModel >= 2 && sdFound()) {
			pergameini.SetInt("GAMESETTINGS", "WIDESCREEN", perGameSettings_wideScreen);
		}
		if ((dsiFeatures() && ms().useBootstrap) || !ms().secondaryDevice) {
			pergameini.SetInt("GAMESETTINGS", "EXTENDED_MEMORY", perGameSettings_expandRomSpace);
		}
	}
	pergameini.SaveIniFile( pergamefilepath );
}

bool checkIfShowAPMsg (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	if (pergameini.GetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 0) == 0) {
		return true;	// Show AP message
	}
	return false;	// Don't show AP message
}

void dontShowAPMsgAgain (std::string filename) {
	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	pergameini.SetInt("GAMESETTINGS", "NO_SHOW_AP_MSG", 1);
	pergameini.SaveIniFile( pergamefilepath );
}

bool checkIfDSiMode (std::string filename) {
	if (ms().secondaryDevice && (!isDSiMode() || !ms().useBootstrap)) {
		return false;
	}

	snprintf(pergamefilepath, sizeof(pergamefilepath), "%s/_nds/TWiLightMenu/gamesettings/%s.ini", (ms().secondaryDevice ? "fat:" : "sd:"), filename.c_str());
	CIniFile pergameini( pergamefilepath );
	perGameSettings_dsiMode = pergameini.GetInt("GAMESETTINGS", "DSI_MODE", (isModernHomebrew[CURPOS] ? true : -1));
	if (perGameSettings_dsiMode == -1) {
		return ms().bstrap_dsiMode;
	} else {
		return perGameSettings_dsiMode;
	}
}

bool showSetDonorRom(u32 arm7size, u32 SDKVersion) {
	if (requiresDonorRom[CURPOS]) return false;

	bool usingB4DS = (!dsiFeatures() && ms().secondaryDevice);
	bool hasCycloDSi = (isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) == 0);

	if (((usingB4DS || hasCycloDSi) && SDKVersion > 0x2000000 && SDKVersion < 0x2008000	// Early SDK2
	 && (arm7size==0x25F70
	  || arm7size==0x25FA4
	  || arm7size==0x2619C
	  || arm7size==0x26A60
	  || arm7size==0x27218
	  || arm7size==0x27224
	  || arm7size==0x2724C))
	 || (SDKVersion >= 0x2008000 && SDKVersion < 0x3000000 && (arm7size==0x26F24 || arm7size==0x26F28))	// Late SDK2
	 || ((usingB4DS || hasCycloDSi) && SDKVersion > 0x3000000 && SDKVersion < 0x4000000	// SDK3
	 && (arm7size==0x28464
	  || arm7size==0x28684
	  || arm7size==0x286A0
	  || arm7size==0x289A4
	  || arm7size==0x289C0
	  || arm7size==0x289F8
	  || arm7size==0x28FFC))
	 || (SDKVersion > 0x5000000 && (arm7size==0x26370 || arm7size==0x2642C || arm7size==0x26488))		// SDK5 (NTR)
	 || ((usingB4DS || hasCycloDSi) && SDKVersion > 0x5000000	// SDK5 (TWL)
	 && (arm7size==0x28F84
	  || arm7size==0x2909C
	  || arm7size==0x2914C
	  || arm7size==0x29164
	  || arm7size==0x29EE8
	  || arm7size==0x2A2EC
	  || arm7size==0x2A318
	  || arm7size==0x2AF18
	  || arm7size==0x2B184
	  || arm7size==0x2B24C
	  || arm7size==0x2C5B4)))
	{
		return true;
	}

	return false;
}

bool donorRomTextShown = false;
void revertDonorRomText(void) {
	if (!donorRomTextShown || setAsDonorRom != STR_DONE) return;

	setAsDonorRom = STR_SET_AS_DONOR_ROM;
}

void perGameSettings (std::string filename) {
	int pressed = 0, held = 0;

	keysSetRepeat(25, 5); // Slow down key repeat

	if (ms().theme == 5) {
		displayGameIcons = false;
	} else if (ms().theme == 4) {
		snd().playStartup();
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		currentBg = 1;
		displayGameIcons = false;
		fadeType = true;
	} else {
		dbox_showIcon = true;
		showdialogbox = true;
	}
	clearText();
	updateText(false);
	
	snprintf (fileCounter, sizeof(fileCounter), "%i/%i", (CURPOS+1)+PAGENUM*40, file_count);
	
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

	bool showSDKVersion = false;
	u32 SDKVersion = 0;
	if (memcmp(gameTid[CURPOS], "HND", 3) == 0 || memcmp(gameTid[CURPOS], "HNE", 3) == 0 || !isHomebrew[CURPOS]) {
		SDKVersion = getSDKVersion(f_nds_file);
		showSDKVersion = true;
	}
	u32 arm9dst = 0;
	u32 arm9size = 0;
	u32 arm7off = 0;
	u32 arm7size = 0;
	u32 romSize = 0;
	u32 pubSize = 0;
	u32 prvSize = 0;
	fseek(f_nds_file, 0x28, SEEK_SET);
	fread(&arm9dst, sizeof(u32), 1, f_nds_file);
	fseek(f_nds_file, 0x2C, SEEK_SET);
	fread(&arm9size, sizeof(u32), 1, f_nds_file);
	fseek(f_nds_file, 0x30, SEEK_SET);
	fread(&arm7off, sizeof(u32), 1, f_nds_file);
	fseek(f_nds_file, 0x3C, SEEK_SET);
	fread(&arm7size, sizeof(u32), 1, f_nds_file);
	fseek(f_nds_file, 0x80, SEEK_SET);
	fread(&romSize, sizeof(u32), 1, f_nds_file);
	fseek(f_nds_file, 0x238, SEEK_SET);
	fread(&pubSize, sizeof(u32), 1, f_nds_file);
	fread(&prvSize, sizeof(u32), 1, f_nds_file);
	fclose(f_nds_file);

	if (romSize > 0) {
		u32 overlaysSize = 0;
		// Calculate overlay pack size
		for (u32 i = 0x4000+arm9size; i < arm7off; i++) {
			overlaysSize++;
		}

		romSize -= arm7off;
		romSize -= arm7size;
		romSize += overlaysSize;
	}

	u32 romSizeLimit = (ms().consoleModel > 0 ? 0x01800000 : 0x800000);
	if (SDKVersion > 0x5000000) {
		romSizeLimit = (ms().consoleModel > 0 ? 0x01000000 : 0);
	}
	u32 romSizeLimit2 = (ms().consoleModel > 0 ? 0x01C00000 : 0xC00000);

	bool showPerGameSettings =
		(!isDSiWare[CURPOS]
		&& memcmp(gameTid[CURPOS], "HND", 3) != 0
		&& memcmp(gameTid[CURPOS], "HNE", 3) != 0);
	if ((dsiFeatures() || !ms().secondaryDevice) && !isHomebrew[CURPOS] && isDSiWare[CURPOS]) {
		showPerGameSettings = true;
	}
	/*if (!ms().useBootstrap && !isHomebrew[CURPOS] && !dsiFeatures()) {
		showPerGameSettings = false;
	}*/
	bool runInShown = false;

	bool showCheats = ((ms().useBootstrap
	/*|| (ms().secondaryDevice && !ms().useBootstrap
		&& ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
		 || (memcmp(io_dldi_data->friendlyName, "R4TF", 4) == 0)
		 || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
		 || (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0)))*/
	|| !ms().secondaryDevice) && !isHomebrew[CURPOS] && !isDSiWare[CURPOS]
	&& memcmp(gameTid[CURPOS], "HND", 3) != 0
	&& memcmp(gameTid[CURPOS], "HNE", 3) != 0);

	firstPerGameOpShown = 0;
	perGameOps = -1;
	for (int i = 0; i < 10; i++) {
		perGameOp[i] = -1;
	}
	if (isHomebrew[CURPOS]) {		// Per-game settings for homebrew
		perGameOps++;
		perGameOp[perGameOps] = 0;	// Language
		if (isModernHomebrew[CURPOS]) {
			perGameOps++;
			perGameOp[perGameOps] = 11;	// Region
		}
		if (!ms().secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 1;	// RAM disk number
		}
		perGameOps++;
		perGameOp[perGameOps] = 6;	// Direct boot
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
		if (dsiFeatures() && ms().consoleModel >= 2 && sdFound() && (gameTid[CURPOS][0] == 'W' || romVersion[CURPOS] == 0x57)) {
			perGameOps++;
			perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
		}
	} else if (showPerGameSettings && isDSiWare[CURPOS]) {	// Per-game settings for DSiWare
		if (ms().dsiWareBooter || ms().consoleModel > 0) {
			perGameOps++;
			perGameOp[perGameOps] = 0;	// Language
			perGameOps++;
			perGameOp[perGameOps] = 11;	// Region
		}
		if (pubSize > 0 || prvSize > 0) {
			perGameOps++;
			perGameOp[perGameOps] = 1;	// Save number
		}
		if (ms().dsiWareBooter || ms().consoleModel > 0) {
			perGameOps++;
			perGameOp[perGameOps] = 7;	// Bootstrap
			showCheats = true;
		}
	} else if (showPerGameSettings) {	// Per-game settings for retail/commercial games
		if (ms().useBootstrap || (dsiFeatures() && unitCode[CURPOS] > 0) || !ms().secondaryDevice) {
			perGameOps++;
			perGameOp[perGameOps] = 0;	// Language
		}
		if ((dsiFeatures() || !ms().secondaryDevice) && unitCode[CURPOS] == 3) {
			perGameOps++;
			perGameOp[perGameOps] = 11;	// Region
		}
		perGameOps++;
		perGameOp[perGameOps] = 1;	// Save number
		if (((dsiFeatures() && (ms().useBootstrap || unitCode[CURPOS] > 0)) || !ms().secondaryDevice)
		&& (unitCode[CURPOS] == 0 || unitCode[CURPOS] == (memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18)==0 ? 2 : 3) || (unitCode[CURPOS] > 0 && !sys().arm7SCFGLocked()))) {
			perGameOps++;
			perGameOp[perGameOps] = 2;	// Run in
			runInShown = true;
		}
		if ((dsiFeatures() || !ms().secondaryDevice) && unitCode[CURPOS] < 3) {
			perGameOps++;
			perGameOp[perGameOps] = 3;	// ARM9 CPU Speed
			perGameOps++;
			perGameOp[perGameOps] = 4;	// VRAM Boost
		}
		if (ms().useBootstrap || (dsiFeatures() && unitCode[CURPOS] > 0) || !ms().secondaryDevice) {
			if (!ms().secondaryDevice) {
				if (unitCode[CURPOS] < 3) {
					perGameOps++;
					perGameOp[perGameOps] = 5;	// Card Read DMA
				}
				if (romSize > romSizeLimit) {
					perGameOps++;
					perGameOp[perGameOps] = 12;	// Async Card Read
				}
			}
			if ((dsiFeatures() || !ms().secondaryDevice)
			 && romSize > romSizeLimit && romSize <= romSizeLimit2+0x80000) {
				perGameOps++;
				perGameOp[perGameOps] = 10;	// Expand ROM space in RAM
			}
			perGameOps++;
			perGameOp[perGameOps] = 7;	// Bootstrap
			if (((dsiFeatures() && sdFound()) || !ms().secondaryDevice) && ms().consoleModel >= 2) {
				perGameOps++;
				perGameOp[perGameOps] = 8;	// Screen Aspect Ratio
			}
			if (showSetDonorRom(arm7size, SDKVersion)) {
				perGameOps++;
				perGameOp[perGameOps] = 9;	// Set as Donor ROM
				donorRomTextShown = true;
			} else {
				donorRomTextShown = false;
			}
		}
	}

	snprintf (gameTIDText, sizeof(gameTIDText), gameTid[CURPOS][0]==0 ? "" : "TID: %s", gameTid[CURPOS]);

	if((SDKVersion > 0x1000000) && (SDKVersion < 0x2000000)) {
		SDKnumbertext = STR_SDK_VER_1;
	} else if((SDKVersion > 0x2000000) && (SDKVersion < 0x3000000)) {
		SDKnumbertext = STR_SDK_VER_2;
	} else if((SDKVersion > 0x3000000) && (SDKVersion < 0x4000000)) {
		SDKnumbertext = STR_SDK_VER_3;
	} else if((SDKVersion > 0x4000000) && (SDKVersion < 0x5000000)) {
		SDKnumbertext = STR_SDK_VER_4;
	} else if((SDKVersion > 0x5000000) && (SDKVersion < 0x6000000)) {
		SDKnumbertext = STR_SDK_VER_5;
	} else {
		SDKnumbertext = STR_SDK_VER_Q;
	}
	if (ms().theme == 5) {
		dbox_showIcon = true;
	} else if (ms().theme == 4) {
		while (!screenFadedIn()) { swiWaitForVBlank(); }
		dbox_showIcon = true;
	} else {
		for (int i = 0; i < 30; i++) { snd().updateStream(); swiWaitForVBlank(); }
	}

	setAsDonorRom = STR_SET_AS_DONOR_ROM;

	// About 35 characters fit in the box.
	dirContName = filename;
	if (strlen(dirContName.c_str()) > 35) {
		// Truncate to 32, 32 + 3 = 35 (because we append "...").
		dirContName.resize(32, ' ');
		size_t first = dirContName.find_first_not_of(' ');
		size_t last = dirContName.find_last_not_of(' ');
		dirContName = dirContName.substr(first, (last - first + 1));
		dirContName.append("...");
	}

	// Test 3DS TWLCFG extraction
	/*FILE* cfgFile = fopen("sd:/TWLCFG.bin", "wb");
	fwrite((void*)0x02000000, 1, 0x4000, cfgFile);
	fclose(cfgFile);*/

	int row1Y = (ms().theme==5 ? 78 : 66);
	if (!showSDKVersion && gameTid[CURPOS][0] == 0) {
		row1Y += 4;
	}
	int row2Y = (ms().theme==5 ? 92 : 80);
	int botRowY = (ms().theme==5 ? 172 : 160);

	while (1) {
		clearText();
		titleUpdate(isDirectory[CURPOS], filename.c_str(), CURPOS);

		int perGameOpYpos = (ms().theme==5 ? 110 : 98);
		Alignment startAlign = ms().rtl() ? Alignment::right : Alignment::left;
		Alignment endAlign = ms().rtl() ? Alignment::left : Alignment::right;
		int perGameOpStartXpos = ms().rtl() ? 256 - 24 : 24;
		int perGameOpEndXpos = ms().rtl() ? 24 : 256 - 24;
		bool flashcardKernelOnly = (!ms().useBootstrap && ms().secondaryDevice && !isHomebrew[CURPOS] && unitCode[CURPOS] == 2 && (perGameSettings_dsiMode==-1 ? !ms().bstrap_dsiMode : perGameSettings_dsiMode==0));

		printSmall(false, ms().rtl() ? 256 - 16 : 16, row1Y, dirContName, startAlign);
		//if (showSDKVersion) printSmall(false, 16, row2Y, (useTwlCfg ? "TwlCfg found!" : SDKnumbertext));
		if (showSDKVersion) printSmall(false, 16, row2Y, SDKnumbertext);
		printSmall(false, 176, row2Y, gameTIDText);
		printSmall(false, 16, botRowY, fileCounter);

		if (showPerGameSettings) {
			printSmall(false, ms().rtl() ? 256 - 16 : 16, perGameOpYpos+(perGameSettings_cursorPosition*14)-(firstPerGameOpShown*14), ms().rtl() ? "<" : ">", startAlign);
		}
		for (int i = firstPerGameOpShown; i < firstPerGameOpShown+4; i++) {
		if (!showPerGameSettings || perGameOp[i] == -1) break;
		switch (perGameOp[i]) {
			case 0:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_LANGUAGE + ":", startAlign);
				if (perGameSettings_language == -1 || flashcardKernelOnly) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_SYSTEM, endAlign);
				} else if (perGameSettings_language == -2) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
				} else if (perGameSettings_language == 0) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_JAPANESE, endAlign);
				} else if (perGameSettings_language == 1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_ENGLISH, endAlign);
				} else if (perGameSettings_language == 2) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_FRENCH, endAlign);
				} else if (perGameSettings_language == 3) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_GERMAN, endAlign);
				} else if (perGameSettings_language == 4) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_ITALIAN, endAlign);
				} else if (perGameSettings_language == 5) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_SPANISH, endAlign);
				} else if (perGameSettings_language == 6) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_CHINESE, endAlign);
				} else if (perGameSettings_language == 7) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_KOREAN, endAlign);
				}
				break;
			case 1:
				if (isHomebrew[CURPOS]) {
					printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_RAM_DISK + ":", startAlign);
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "%i", perGameSettings_ramDiskNo);
				} else {
					printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_SAVE_NO + ":", startAlign);
					snprintf (saveNoDisplay, sizeof(saveNoDisplay), "%i", perGameSettings_saveNo);
				}
				if (isHomebrew[CURPOS] && perGameSettings_ramDiskNo == -1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_NONE, endAlign);
				} else {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, saveNoDisplay, endAlign);
				}
				break;
			case 2:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_RUN_IN + ":", startAlign);
				if (perGameSettings_dsiMode == -1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
				} else if (perGameSettings_dsiMode == 2) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DSI_MODE_FORCED, endAlign);
				} else if (perGameSettings_dsiMode == 1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DSI_MODE, endAlign);
				} else {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DS_MODE, endAlign);
				}
				break;
			case 3:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_ARM9_CPU_SPEED + ":", startAlign);
				if ((perGameSettings_dsiMode==-1 ? ms().bstrap_dsiMode : perGameSettings_dsiMode > 0) && runInShown) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, "133mhz (TWL)", endAlign);
				} else {
					if (perGameSettings_boostCpu == -1) {
						printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
					} else if (perGameSettings_boostCpu == 1) {
						printSmall(false, perGameOpEndXpos, perGameOpYpos, "133mhz (TWL)", endAlign);
					} else {
						printSmall(false, perGameOpEndXpos, perGameOpYpos, "67mhz (NTR)", endAlign);
					}
				}
				break;
			case 4:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_VRAM_BOOST + ":", startAlign);
				if ((perGameSettings_dsiMode==-1 ? ms().bstrap_dsiMode : perGameSettings_dsiMode > 0) && runInShown) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_ON, endAlign);
				} else {
					if (perGameSettings_boostVram == -1) {
						printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
					} else if (perGameSettings_boostVram == 1) {
						printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_ON, endAlign);
					} else {
						printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_OFF, endAlign);
					}
				}
				break;
			case 5:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_CARD_READ_DMA + ":", startAlign);
				if (perGameSettings_cardReadDMA == -1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
				} else if (perGameSettings_cardReadDMA == 1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_ON, endAlign);
				} else {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_OFF, endAlign);
				}
				break;
			case 6:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_DIRECT_BOOT + ":", startAlign);
				if (perGameSettings_directBoot) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_YES, endAlign);
				} else {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_NO, endAlign);
				}
				break;
			case 7:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, ms().rtl() ? ":Bootstrap" : "Bootstrap:", startAlign);
				if (flashcardKernelOnly) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_NOT_USED, endAlign);
				} else if (perGameSettings_bootstrapFile == -1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
				} else if (perGameSettings_bootstrapFile == 1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_NIGHTLY, endAlign);
				} else {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_RELEASE, endAlign);
				}
				break;
			case 8:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_SCREEN_ASPECT_RATIO + ":", startAlign);
				if (flashcardKernelOnly) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_NOT_USED, endAlign);
				} else if (perGameSettings_wideScreen == -1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
				} else if (perGameSettings_wideScreen == 1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, "16:10", endAlign);
				} else {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, "4:3", endAlign);
				}
				break;
			case 9:
				printSmall(false, 0, perGameOpYpos, setAsDonorRom, Alignment::center);
				break;
			case 10:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_EXPAND_ROM_SPACE + ":", startAlign);
				if (flashcardKernelOnly) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_NOT_USED, endAlign);
				} else if (perGameSettings_expandRomSpace == -1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
				} else if (perGameSettings_expandRomSpace == 2) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_Y_512KB, endAlign);
				} else if (perGameSettings_expandRomSpace == 1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_YES, endAlign);
				} else {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_NO, endAlign);
				}
				break;
			case 11:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_REGION + ":", startAlign);
				if (perGameSettings_region == -3) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
				} else if (perGameSettings_region == -2) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_GAME, endAlign);
				} else if (perGameSettings_region == -1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_SYSTEM, endAlign);
				} else if (perGameSettings_region == 0) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_JAPAN, endAlign);
				} else if (perGameSettings_region == 1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_USA, endAlign);
				} else if (perGameSettings_region == 2) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_EUROPE, endAlign);
				} else if (perGameSettings_region == 3) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_AUSTRALIA, endAlign);
				} else if (perGameSettings_region == 4) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_CHINA, endAlign);
				} else if (perGameSettings_region == 5) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_KOREA, endAlign);
				}
				break;
			case 12:
				printSmall(false, perGameOpStartXpos, perGameOpYpos, STR_ASYNCH_CARD_READ + ":", startAlign);
				if (perGameSettings_asyncCardRead == -1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_DEFAULT, endAlign);
				} else if (perGameSettings_asyncCardRead == 1) {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_ON, endAlign);
				} else {
					printSmall(false, perGameOpEndXpos, perGameOpYpos, STR_OFF, endAlign);
				}
				break;
		}
		perGameOpYpos += 14;
		}
		if (!showPerGameSettings && !showCheats) {
			printSmall(false, 240, botRowY, STR_A_OK, Alignment::right);
		} else {
			printSmall(false, 240, botRowY, showCheats ? STR_X_CHEATS_B_BACK : STR_B_BACK, Alignment::right);
		}
		updateText(false);
		do {
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
			checkSdEject();
			tex().drawVolumeImageCached();
			tex().drawBatteryImageCached();
			drawCurrentTime();
			drawCurrentDate();
			drawClockColon();
			snd().updateStream();
			swiWaitForVBlank();
		} while (!held);

		if (!showPerGameSettings) {
			if ((pressed & KEY_A) || (pressed & KEY_B)) {
				snd().playBack();
				break;
			}
		} else {
			if (held & KEY_UP) {
				snd().playSelect();
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
				snd().playSelect();
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
						if (isHomebrew[CURPOS]) {
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
						if (perGameSettings_dsiMode == 1 && unitCode[CURPOS] == 0) perGameSettings_dsiMode--;
						else if (perGameSettings_dsiMode == 2 && unitCode[CURPOS] > 0) perGameSettings_dsiMode--;
						if (perGameSettings_dsiMode < -1) perGameSettings_dsiMode = 2-isHomebrew[CURPOS];
						break;
					case 3:
						if (perGameSettings_dsiMode < 1 || !runInShown) {
							perGameSettings_boostCpu--;
							if (perGameSettings_boostCpu < -1) perGameSettings_boostCpu = 1;
						}
						break;
					case 4:
						if (perGameSettings_dsiMode < 1 || !runInShown) {
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
						perGameSettings_wideScreen--;
						if (perGameSettings_wideScreen < -1) perGameSettings_wideScreen = 1;
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
						if (!dsiFeatures()) {
							if (perGameSettings_region == -1) {
								perGameSettings_region--;
							}
							if (perGameSettings_region == -2) {
								perGameSettings_region--;
							}
						}
						if (perGameSettings_region < -3) perGameSettings_region = 5;
						break;
					case 12:
						perGameSettings_asyncCardRead--;
						if (perGameSettings_asyncCardRead < -1) perGameSettings_asyncCardRead = 1;
						break;
				}
				(ms().theme == 4) ? snd().playLaunch() : snd().playSelect();
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
						if (isHomebrew[CURPOS]) {
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
						if (perGameSettings_dsiMode == 1 && unitCode[CURPOS] == 0) perGameSettings_dsiMode++;
						else if (perGameSettings_dsiMode == 2 && unitCode[CURPOS] > 0) perGameSettings_dsiMode++;
						if (perGameSettings_dsiMode > 2-isHomebrew[CURPOS]) perGameSettings_dsiMode = -1;
						break;
					case 3:
						if (perGameSettings_dsiMode < 1 || !runInShown) {
							perGameSettings_boostCpu++;
							if (perGameSettings_boostCpu > 1) perGameSettings_boostCpu = -1;
						}
						break;
					case 4:
						if (perGameSettings_dsiMode < 1 || !runInShown) {
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
						const char* pathDefine = "DONOR_NDS_PATH";
						if (SDKVersion > 0x2000000 && SDKVersion < 0x2008000) {
							pathDefine = "DONORE2_NDS_PATH";
						} else if (SDKVersion > 0x2008000 && SDKVersion < 0x3000000) {
							pathDefine = "DONOR2_NDS_PATH";
						} else if (SDKVersion > 0x3000000 && SDKVersion < 0x5000000) {
							pathDefine = "DONOR3_NDS_PATH";
						} else if (unitCode[CURPOS] > 0 && SDKVersion > 0x5000000) {
							pathDefine = "DONORTWL_NDS_PATH";
						}
						std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
						RemoveTrailingSlashes(romFolderNoSlash);
						bootstrapinipath = ((!ms().secondaryDevice || (isDSiMode() && sdFound())) ? "sd:/_nds/nds-bootstrap.ini" : "fat:/_nds/nds-bootstrap.ini");
						CIniFile bootstrapini(bootstrapinipath);
						bootstrapini.SetString("NDS-BOOTSTRAP", pathDefine, romFolderNoSlash+"/"+filename);
						bootstrapini.SaveIniFile(bootstrapinipath);
						setAsDonorRom = STR_DONE;
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
						if (!dsiFeatures()) {
							if (perGameSettings_region == -2) {
								perGameSettings_region++;
							}
							if (perGameSettings_region == -1) {
								perGameSettings_region++;
							}
						}
						if (perGameSettings_region > 5) perGameSettings_region = -3;
						break;
					case 12:
						perGameSettings_asyncCardRead++;
						if (perGameSettings_asyncCardRead > 1) perGameSettings_asyncCardRead = -1;
						break;
				}
				(ms().theme == 4) ? snd().playLaunch() : snd().playSelect();
				perGameSettingsChanged = true;
			}

			if (pressed & KEY_B) {
				snd().playBack();
				if (perGameSettingsChanged) {
					savePerGameSettings(filename);
					perGameSettingsChanged = false;
				}
				break;
			}
		}
		if ((pressed & KEY_X) && !isHomebrew[CURPOS] && showCheats) {
			(ms().theme == 4) ? snd().playLaunch() : snd().playSelect();
			CheatCodelist codelist;
			codelist.selectCheats(filename);
		}
	}
	showdialogbox = false;
	if (ms().theme == 5) {
		displayGameIcons = true;
		dbox_showIcon = false;
	} else if (ms().theme == 4) {
		fadeType = false;	   // Fade to black
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
		clearText();
		currentBg = 0;
		displayGameIcons = true;
		fadeType = true;
		snd().playStartup();
	} else {
		clearText();
	}
	updateText(false);

	keysSetRepeat(10, 2); // Reset key repeat
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
