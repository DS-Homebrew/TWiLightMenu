#include <nds.h>
#include <nds/arm9/dldi.h>
#include "fat_ext.h"
#include "io_m3_common.h"
#include "io_g6_common.h"
#include "io_sc_common.h"
#include "myDSiMode.h"
#include "exptools.h"
#include "nand/nandio.h"

#include "bootsplash.h"
#include "bootstrapsettings.h"
#include "common/bootstrappaths.h"
#include "common/cardlaunch.h"
#include "common/dsimenusettings.h"
#include "common/fileCopy.h"
#include "common/flashcard.h"
#include "common/inifile.h"
#include "common/nds_loader_arm9.h"
#include "common/nitrofs.h"
#include "ndsheaderbanner.h"
#include "common/pergamesettings.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "tool/stringtool.h"
#include "graphics/graphics.h"
#include "twlmenuppvideo.h"
#include "language.h"
#include "graphics/fontHandler.h"
#include "sound.h"

#include "autoboot.h"

#include "twlClockExcludeMap.h"
#include "dmaExcludeMap.h"
#include "asyncReadExcludeMap.h"
#include "saveMap.h"
#include "ROMList.h"

#include "sr_data_srllastran.h"		 // For rebooting into the game

bool useTwlCfg = false;

bool renderScreens = false;
bool fadeType = false; // false = out, true = in
bool fadeColor = true; // false = black, true = white
extern bool controlTopBright;
extern bool controlBottomBright;

//bool soundfreqsettingChanged = false;
bool hiyaAutobootFound = false;
//static int flashcard;
/* Flashcard value
	0: DSTT/R4i Gold/R4i-SDHC/R4 SDHC Dual-Core/R4 SDHC Upgrade/SC DSONE
	1: R4DS (Original Non-SDHC version)/ M3 Simply
	2: R4iDSN/R4i Gold RTS/R4 Ultra
	3: Acekard 2(i)/Galaxy Eagle/M3DS Real
	4: Acekard RPG
	5: Ace 3DS+/Gateway Blue Card/R4iTT
	6: SuperCard DSTWO
*/

const char *hiyacfwinipath = "sd:/hiya/settings.ini";
const char *settingsinipath = DSIMENUPP_INI;

static sNDSHeaderExt NDSHeader;

/**
 * Remove trailing slashes from a pathname, if present.
 * @param path Pathname to modify.
 */
void RemoveTrailingSlashes(std::string& path)
{
	while (!path.empty() && path[path.size()-1] == '/') {
		path.resize(path.size()-1);
	}
}

static const std::string slashchar = "/";
static const std::string woodfat = "fat0:/";
static const std::string dstwofat = "fat1:/";

typedef TWLSettings::TLaunchType Launch;

int screenmode = 0;
int subscreenmode = 0;

touchPosition touch;

using namespace std;

//---------------------------------------------------------------------------------
void stop(void)
{
	//---------------------------------------------------------------------------------
	while (1)
	{
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

/*//---------------------------------------------------------------------------------
void doPause(void)
{
	//---------------------------------------------------------------------------------
	printf("Press start...\n");
	//printSmall(false, x, y, "Press start...");
	while (1)
	{
		scanKeys();
		if (keysDown() & KEY_START)
			break;
	}
	scanKeys();
}*/

void loadMainMenu()
{
	fadeColor = true;
	controlTopBright = true;
	controlBottomBright = true;
	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	if (!isDSiMode()) {
		chdir("fat:/");
	}
	runNdsFile("/_nds/TWiLightMenu/mainmenu.srldr", 0, NULL, true, false, false, true, true, -1);
}

void loadROMselect(void)
{
	fadeColor = true;
	controlTopBright = true;
	controlBottomBright = true;
	fadeType = false;
	fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out

	if (!isDSiMode()) {
		chdir("fat:/");
	}
	switch (ms().theme) {
		/*case 3:
			runNdsFile("/_nds/TWiLightMenu/akmenu.srldr", 0, NULL, true, false, false, true, true, -1);
			break;*/
		case 2:
		case 6:
			runNdsFile("/_nds/TWiLightMenu/r4menu.srldr", 0, NULL, true, false, false, true, true, -1);
			break;
		default:
			runNdsFile("/_nds/TWiLightMenu/dsimenu.srldr", 0, NULL, true, false, false, true, true, -1);
			break;
	}
	stop();
}

/*void loadROMselectAsynch(void)
{
	switch (ms().theme) {
		case 2:
		case 6:
			loadNds9iAsynch(!isDSiMode() ? "fat:/_nds/TWiLightMenu/r4menu.srldr" : "/_nds/TWiLightMenu/r4menu.srldr");
			break;
		default:
			loadNds9iAsynch(!isDSiMode() ? "fat:/_nds/TWiLightMenu/dsimenu.srldr" : "/_nds/TWiLightMenu/dsimenu.srldr");
			break;
	}
}*/

bool extention(const std::string& filename, const char* ext) {
	if(strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext)) {
		return false;
	} else {
		return true;
	}
}

// Unlaunch needs the path in 16 bit unicode so this function is
// from the FontGraphic class, but as that's not used in title
// it's copied here. Remove this if adding FontGraphic to title.
std::u16string utf8to16(std::string_view text) {
	std::u16string out;
	for(uint i=0;i<text.size();) {
		char16_t c;
		if(!(text[i] & 0x80)) {
			c = text[i++];
		} else if((text[i] & 0xE0) == 0xC0) {
			c  = (text[i++] & 0x1F) << 6;
			c |=  text[i++] & 0x3F;
		} else if((text[i] & 0xF0) == 0xE0) {
			c  = (text[i++] & 0x0F) << 12;
			c |= (text[i++] & 0x3F) << 6;
			c |=  text[i++] & 0x3F;
		} else {
			i++; // out of range or something (This only does up to 0xFFFF since it goes to a U16 anyways)
		}
		out += c;
	}
	return out;
}

void unlaunchRomBoot(std::string_view rom) {
	std::u16string path(utf8to16(rom));
	if (path.substr(0, 3) == u"sd:") {
		path = u"sdmc:" + path.substr(3);
	}

	tonccpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;		// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) = 0;			// Unlaunch Flags
	*(u32*)(0x02000810) |= BIT(0);		// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);		// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;		// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;		// Unlaunch Lower screen BG color (0..7FFFh)
	toncset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);		// Unlaunch Reserved (zero)
	for (uint i = 0; i < std::min(path.length(), 0x103u); i++) {
		((char16_t*)0x02000838)[i] = path[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
	}
	while (*(u16*)(0x0200080E) == 0) {	// Keep running, so that CRC16 isn't 0
		*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16
	}

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_08, 1);	// Reboot into DSiWare title, booted via Unlaunch
	stop();
}

/**
 * Reboot into an SD game when in DS mode.
 */
void ntrStartSdGame(void) {
	if (ms().consoleModel == 0) {
		unlaunchRomBoot("sd:/_nds/TWiLightMenu/resetgame.srldr");
	} else {
		tonccpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
		DC_FlushAll();						// Make reboot not fail
		fifoSendValue32(FIFO_USER_08, 1);
		stop();
	}
}

void dsCardLaunch() {
	*(u32 *)(0x02000300) = 0x434E4C54; // Set "CNLT" warmboot flag
	*(u16 *)(0x02000304) = 0x1801;
	*(u32 *)(0x02000308) = 0x43415254; // "CART"
	*(u32 *)(0x0200030C) = 0x00000000;
	*(u32 *)(0x02000310) = 0x43415254; // "CART"
	*(u32 *)(0x02000314) = 0x00000000;
	*(u32 *)(0x02000318) = 0x00000013;
	*(u32 *)(0x0200031C) = 0x00000000;
	*(u16 *)(0x02000306) = swiCRC16(0xFFFF, (void *)0x02000308, 0x18);

	unlaunchSetHiyaBoot();

	DC_FlushAll();						// Make reboot not fail
	fifoSendValue32(FIFO_USER_08, 1); // Reboot into DSiWare title, booted via Launcher
	stop();
}

void s2RamAccess(bool open) {
	if (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS) return;

	if (open) {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode(M3_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation(G6_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode(SC_MODE_RAM);
		}
	} else {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode(M3_MODE_MEDIA);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation(G6_MODE_MEDIA);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode(SC_MODE_MEDIA);
		}
	}
}

void gbaSramAccess(bool open) {
	if (open) {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode(M3_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation(G6_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode(SC_MODE_RAM_RO);
		}
	} else {
		if (*(u16*)(0x020000C0) == 0x334D) {
			_M3_changeMode((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? M3_MODE_MEDIA : M3_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x3647) {
			_G6_SelectOperation((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? G6_MODE_MEDIA : G6_MODE_RAM);
		} else if (*(u16*)(0x020000C0) == 0x4353) {
			_SC_changeMode((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) ? SC_MODE_MEDIA : SC_MODE_RAM);
		}
	}
}

void lastRunROM()
{
	/*fifoSendValue32(FIFO_USER_01, 1); // Fade out sound
	for (int i = 0; i < 25; i++)
		swiWaitForVBlank();
	fifoSendValue32(FIFO_USER_01, 0); // Cancel sound fade out*/

	std::string romfolder = ms().romPath[ms().previousUsedDevice];
	while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
		romfolder.resize(romfolder.size()-1);
	}
	chdir(romfolder.c_str());

	std::string filename = ms().romPath[ms().previousUsedDevice];
	const size_t last_slash_idx = filename.find_last_of("/");
	if (std::string::npos != last_slash_idx)
	{
		filename.erase(0, last_slash_idx + 1);
	}

	vector<char *> argarray;
	if (ms().launchType[ms().previousUsedDevice] > Launch::EDSiWareLaunch)
	{
		argarray.push_back(strdup("null"));
		if (ms().launchType[ms().previousUsedDevice] == Launch::EGBANativeLaunch) {
			argarray.push_back(strdup(ms().romPath[ms().previousUsedDevice].c_str()));
		} else {
			argarray.push_back(strdup(ms().homebrewArg[ms().previousUsedDevice].c_str()));
		}
	}

	int err = 0;
	if (ms().slot1Launched && !flashcardFound())
	{
		if (ms().slot1LaunchMethod==0 || sys().arm7SCFGLocked()) {
			dsCardLaunch();
		} else if (ms().slot1LaunchMethod==2) {
			unlaunchRomBoot("cart:");
		} else {
			if ((ms().consoleModel >= 2 && !ms().wideScreen) || ms().consoleModel < 2 || ms().macroMode) {
				remove("/_nds/nds-bootstrap/wideCheatData.bin");
			} else if (access("sd:/_nds/nds-bootstrap/wideCheatData.bin", F_OK) == 0 && (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0)) {
				if (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) {
					rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak");
				}
				if (rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
					tonccpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
					DC_FlushAll();
					fifoSendValue32(FIFO_USER_08, 1);
					stop();
				}
			}
			err = runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true, -1);
		}
	}
	if (ms().launchType[ms().previousUsedDevice] == Launch::ESDFlashcardLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		loadPerGameSettings(filename);

		bool dsiBinariesFound = true;
		char game_TID[5];
		u8 unitCode = 0;

		FILE *f_nds_file = fopen(filename.c_str(), "rb");
		dsiBinariesFound = checkDsiBinaries(f_nds_file);
		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		fseek(f_nds_file, 0x12, SEEK_SET);
		fread(&unitCode, 1, 1, f_nds_file);
		game_TID[4] = 0;

		fclose(f_nds_file);

		if (ms().useBootstrap || !ms().previousUsedDevice || (dsiFeatures() && unitCode > 0 && (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode))
		|| ((game_TID[0] == 'K' || game_TID[0] == 'Z') && unitCode > 0))
		{
			std::string savepath;

			bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);
			bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);
			bool boostCpu = (perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu);
			bool cardReadDMA = (perGameSettings_cardReadDMA == -1 ? DEFAULT_CARD_READ_DMA : perGameSettings_cardReadDMA);
			bool asyncCardRead = (perGameSettings_asyncCardRead == -1 ? DEFAULT_ASYNC_CARD_READ : perGameSettings_asyncCardRead);
			bool swiHaltHook = (perGameSettings_swiHaltHook == -1 ? DEFAULT_SWI_HALT_HOOK : perGameSettings_swiHaltHook);

			if (ms().homebrewBootstrap) {
				argarray.push_back((char*)(useNightly ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds"));
			} else {
				if ((ms().consoleModel >= 2 && !useWidescreen) || ms().consoleModel < 2 || ms().macroMode) {
					remove("/_nds/nds-bootstrap/wideCheatData.bin");
				} else if ((access(ms().previousUsedDevice ? "fat:/_nds/nds-bootstrap/wideCheatData.bin" : "sd:/_nds/nds-bootstrap/wideCheatData.bin", F_OK) == 0) && (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0)) {
					if (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) {
						rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak");
					}
					if (rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
						tonccpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
						DC_FlushAll();
						fifoSendValue32(FIFO_USER_08, 1);
						stop();
					}
				}

				const char *typeToReplace = ".nds";
				if (extention(filename, ".dsi")) {
					typeToReplace = ".dsi";
				} else if (extention(filename, ".ids")) {
					typeToReplace = ".ids";
				} else if (extention(filename, ".srl")) {
					typeToReplace = ".srl";
				} else if (extention(filename, ".app")) {
					typeToReplace = ".app";
				}

				std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
				std::string romFolderNoSlash = romfolder;
				RemoveTrailingSlashes(romFolderNoSlash);
				mkdir ("saves", 0777);
				savepath = romFolderNoSlash+"/saves/"+savename;
				if (ms().previousUsedDevice && ms().fcSaveOnSd) {
					savepath = replaceAll(savepath, "fat:/", "sd:/");
				}

				if ((strcmp(game_TID, "####") != 0) && (strncmp(game_TID, "NTR", 3) != 0)) {
					u32 orgsavesize = getFileSize(savepath.c_str());
					u32 savesize = 524288;	// 512KB (default size for most games)

					u32 gameTidHex = 0;
					tonccpy(&gameTidHex, &game_TID, 4);

					for (int i = 0; i < (int)sizeof(ROMList)/12; i++) {
						ROMListEntry* curentry = &ROMList[i];
						if (gameTidHex == curentry->GameCode) {
							if (curentry->SaveMemType != 0xFFFFFFFF) savesize = sramlen[curentry->SaveMemType];
							break;
						}
					}

					if ((orgsavesize == 0 && savesize > 0) || (orgsavesize < savesize)) {
						consoleDemoInit();
						iprintf((orgsavesize == 0) ? "Creating save file...\n" : "Expanding save file...\n");
						iprintf ("\n");
						fadeType = true;

						FILE *pFile = fopen(savepath.c_str(), orgsavesize > 0 ? "r+" : "wb");
						if (pFile) {
							fseek(pFile, savesize - 1, SEEK_SET);
							fputc('\0', pFile);
							fclose(pFile);
						}
						iprintf((orgsavesize == 0) ? "Save file created!\n" : "Save file expanded!\n");

						for (int i = 0; i < 30; i++) {
							swiWaitForVBlank();
						}
						fadeType = false;
						for (int i = 0; i < 25; i++) {
							swiWaitForVBlank();
						}
					}

					if (!ms().ignoreBlacklists) {
						// TODO: If the list gets large enough, switch to bsearch().
						for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
							if (memcmp(game_TID, twlClockExcludeList[i], 3) == 0) {
								// Found match
								boostCpu = false;
								break;
							}
						}

						// TODO: If the list gets large enough, switch to bsearch().
						for (unsigned int i = 0; i < sizeof(cardReadDMAExcludeList)/sizeof(cardReadDMAExcludeList[0]); i++) {
							if (memcmp(game_TID, cardReadDMAExcludeList[i], 3) == 0) {
								// Found match
								cardReadDMA = false;
								break;
							}
						}

						// TODO: If the list gets large enough, switch to bsearch().
						for (unsigned int i = 0; i < sizeof(asyncReadExcludeList)/sizeof(asyncReadExcludeList[0]); i++) {
							if (memcmp(game_TID, asyncReadExcludeList[i], 3) == 0) {
								// Found match
								asyncCardRead = false;
								break;
							}
						}
					}
				}

				if (sdFound() && !ms().previousUsedDevice) {
					argarray.push_back((char*)(useNightly ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds"));
				} else {
					argarray.push_back((char*)(useNightly ? "/_nds/nds-bootstrap-nightly.nds" : "/_nds/nds-bootstrap-release.nds"));
				}
			}
			if (ms().previousUsedDevice || !ms().homebrewBootstrap) {
				CIniFile bootstrapini( sdFound() ? BOOTSTRAP_INI_SD : BOOTSTRAP_INI_FC );
				bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().romPath[ms().previousUsedDevice]);
				bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
				bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
				bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", (perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", (perGameSettings_region == -3 ? ms().gameRegion : perGameSettings_region));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", !dsiBinariesFound ? 0 : (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", (perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "CARD_READ_DMA", cardReadDMA);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "ASYNC_CARD_READ", asyncCardRead);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "SWI_HALT_HOOK", swiHaltHook);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "EXTENDED_MEMORY", perGameSettings_expandRomSpace == -1 ? ms().extendedMemory : perGameSettings_expandRomSpace);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 
					(ms().forceSleepPatch
				|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !sys().isRegularDS())
				|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !sys().isRegularDS())
				|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !sys().isRegularDS())
				|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS()))
				);
				bootstrapini.SaveIniFile( sdFound() ? BOOTSTRAP_INI_SD : BOOTSTRAP_INI_FC );
			}
			err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], (ms().homebrewBootstrap ? false : true), true, false, true, true, -1);
		}
		else
		{
			bool runNds_boostCpu = false;
			bool runNds_boostVram = false;
			loadPerGameSettings(filename);
			if (REG_SCFG_EXT != 0) {
				runNds_boostCpu = perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
				runNds_boostVram = perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram;
			}

			// Move .sav outside of "saves" folder for flashcard kernel usage
			const char *typeToReplace = ".nds";
			if (extention(filename, ".dsi")) {
				typeToReplace = ".dsi";
			} else if (extention(filename, ".ids")) {
				typeToReplace = ".ids";
			} else if (extention(filename, ".srl")) {
				typeToReplace = ".srl";
			} else if (extention(filename, ".app")) {
				typeToReplace = ".app";
			}

			std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
			std::string savenameFc = replaceAll(filename, typeToReplace, ".sav");
			std::string romFolderNoSlash = romfolder;
			RemoveTrailingSlashes(romFolderNoSlash);
			mkdir("saves", 0777);
			std::string savepath = romFolderNoSlash + "/saves/" + savename;
			std::string savepathFc = romFolderNoSlash + "/" + savenameFc;
			rename(savepath.c_str(), savepathFc.c_str());

			std::string fcPath;
			if ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "R4TF", 4) == 0)
			 || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)) {
				CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
				fcPath = replaceAll(ms().romPath[ms().previousUsedDevice], "fat:/", woodfat);
				fcrompathini.SetString("Save Info", "lastLoaded", fcPath);
				fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
				err = runNdsFile("fat:/Wfwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, -1);
			} else if (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0) {
				CIniFile fcrompathini("fat:/_afwd/lastsave.ini");
				fcPath = replaceAll(ms().romPath[ms().previousUsedDevice], "fat:/", woodfat);
				fcrompathini.SetString("Save Info", "lastLoaded", fcPath);
				fcrompathini.SaveIniFile("fat:/_afwd/lastsave.ini");
				err = runNdsFile("fat:/Afwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, -1);
			} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
				CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
				fcPath = replaceAll(ms().romPath[ms().previousUsedDevice], "fat:/", dstwofat);
				fcrompathini.SetString("Dir Info", "fullName", fcPath);
				fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
				err = runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, -1);
			} else if ((memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0)
					 || (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0)
					 || (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0)
					 || (memcmp(io_dldi_data->friendlyName, "DSONE", 5) == 0)
					 || (memcmp(io_dldi_data->friendlyName, "M3DS DLDI", 9) == 0)) {
				CIniFile fcrompathini("fat:/TTMenu/YSMenu.ini");
				fcPath = replaceAll(ms().romPath[ms().previousUsedDevice], "fat:/", slashchar);
				fcrompathini.SetString("YSMENU", "AUTO_BOOT", fcPath);
				fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
				err = runNdsFile("fat:/YSMenu.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, -1);
			}
		}
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::ESDFlashcardDirectLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.push_back((char*)ms().romPath[ms().previousUsedDevice].c_str());

		char game_TID[5];

		FILE *f_nds_file = fopen(filename.c_str(), "rb");
		fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
		fread(game_TID, 1, 4, f_nds_file);
		game_TID[4] = 0;

		fclose(f_nds_file);

		loadPerGameSettings(filename);

		int language = perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language;
		int gameRegion = perGameSettings_region == -3 ? ms().gameRegion : perGameSettings_region;

		// Set region flag
		if (gameRegion == -2 && game_TID[3] != 'A' && game_TID[3] != '#') {
			if (game_TID[3] == 'J') {
				*(u8*)(0x02FFFD70) = 0;
			} else if (game_TID[3] == 'E' || game_TID[3] == 'T') {
				*(u8*)(0x02FFFD70) = 1;
			} else if (game_TID[3] == 'P' || game_TID[3] == 'V') {
				*(u8*)(0x02FFFD70) = 2;
			} else if (game_TID[3] == 'U') {
				*(u8*)(0x02FFFD70) = 3;
			} else if (game_TID[3] == 'C') {
				*(u8*)(0x02FFFD70) = 4;
			} else if (game_TID[3] == 'K') {
				*(u8*)(0x02FFFD70) = 5;
			}
		} else if (gameRegion == -1 || (gameRegion == -2 && (game_TID[3] == 'A' || game_TID[3] == '#'))) {
		  if (useTwlCfg) {
			u8 country = *(u8*)0x02000405;
			if (country == 0x01) {
				*(u8*)(0x02FFFD70) = 0;	// Japan
			} else if (country == 0xA0) {
				*(u8*)(0x02FFFD70) = 4;	// China
			} else if (country == 0x88) {
				*(u8*)(0x02FFFD70) = 5;	// Korea
			} else if (country == 0x41 || country == 0x5F) {
				*(u8*)(0x02FFFD70) = 3;	// Australia
			} else if ((country >= 0x08 && country <= 0x34) || country == 0x99 || country == 0xA8) {
				*(u8*)(0x02FFFD70) = 1;	// USA
			} else if (country >= 0x40 && country <= 0x70) {
				*(u8*)(0x02FFFD70) = 2;	// Europe
			}
		  } else {
			u8 consoleType = 0;
			readFirmware(0x1D, &consoleType, 1);
			if (consoleType == 0x43 || consoleType == 0x63) {
				*(u8*)(0x02FFFD70) = 4;	// China
			} else if (PersonalData->language == 0) {
				*(u8*)(0x02FFFD70) = 0;	// Japan
			} else if (PersonalData->language == 1 /*|| PersonalData->language == 2 || PersonalData->language == 5*/) {
				*(u8*)(0x02FFFD70) = 1;	// USA
			} else /*if (PersonalData->language == 3 || PersonalData->language == 4)*/ {
				*(u8*)(0x02FFFD70) = 2;	// Europe
			} /*else {
				*(u8*)(0x02FFFD70) = 5;	// Korea
			}*/
		  }
		} else {
			*(u8*)(0x02FFFD70) = gameRegion;
		}

		if (useTwlCfg && language >= 0 && language <= 7 && *(u8*)0x02000406 != language) {
			tonccpy((char*)0x02000600, (char*)0x02000400, 0x200);
			*(u8*)0x02000606 = language;
			*(u32*)0x02FFFDFC = 0x02000600;
		}

		bool runNds_boostCpu = false;
		bool runNds_boostVram = false;

		bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

		if (ms().consoleModel >= 2 && useWidescreen && ms().homebrewHasWide) {
			//argarray.push_back((char*)"wide");
			if (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) {
				rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak");
			}
			if (rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
				tonccpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
				DC_FlushAll();
				fifoSendValue32(FIFO_USER_08, 1);
				stop();
			}
		}

		runNds_boostCpu = perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
		runNds_boostVram = perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram;

		err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, (!perGameSettings_dsiMode ? true : false), runNds_boostCpu, runNds_boostVram, language);
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EDSiWareLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		if (ms().dsiWareBooter || sys().arm7SCFGLocked() || ms().consoleModel > 0)
		{
			if (ms().homebrewBootstrap) {
				unlaunchRomBoot(ms().previousUsedDevice ? "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath);
			} else {
				loadPerGameSettings(filename);

				bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);
				bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);

				const char *typeToReplace = ".nds";
				if (extention(filename, ".dsi")) {
					typeToReplace = ".dsi";
				} else if (extention(filename, ".ids")) {
					typeToReplace = ".ids";
				} else if (extention(filename, ".srl")) {
					typeToReplace = ".srl";
				} else if (extention(filename, ".app")) {
					typeToReplace = ".app";
				}

				std::string romFolderNoSlash = romfolder;
				RemoveTrailingSlashes(romFolderNoSlash);
				mkdir ("saves", 0777);

				FILE *f_nds_file = fopen(filename.c_str(), "rb");

				fread(&NDSHeader, 1, sizeof(NDSHeader), f_nds_file);
				fclose(f_nds_file);

				ms().dsiWareSrlPath = ms().romPath[ms().previousUsedDevice];
				ms().dsiWarePubPath = romFolderNoSlash + "/saves/" + filename;
				ms().dsiWarePubPath = replaceAll(ms().dsiWarePubPath, typeToReplace, (strncmp(NDSHeader.gameCode, "Z2E", 3) == 0 && ms().previousUsedDevice && (!sdFound() || !ms().dsiWareToSD)) ? getSavExtension() : getPubExtension());
				ms().dsiWarePrvPath = romFolderNoSlash + "/saves/" + filename;
				ms().dsiWarePrvPath = replaceAll(ms().dsiWarePrvPath, typeToReplace, getPrvExtension());
				ms().saveSettings();

				if ((getFileSize(ms().dsiWarePubPath.c_str()) == 0) && (NDSHeader.pubSavSize > 0)) {
					consoleDemoInit();
					iprintf("Creating public save file...\n");
					iprintf ("\n");
					fadeType = true;

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					toncset(buffer, 0, sizeof(buffer));
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%X.savhdr",
						 (unsigned int)NDSHeader.pubSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile)
						fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(ms().dsiWarePubPath.c_str(), "wb");
					if (pFile) {
						fwrite(buffer, 1, sizeof(buffer), pFile);
						fseek(pFile, NDSHeader.pubSavSize - 1, SEEK_SET);
						fputc('\0', pFile);
						fclose(pFile);
					}
					iprintf("Public save file created!\n");

					for (int i = 0; i < 30; i++) {
						swiWaitForVBlank();
					}
					fadeType = false;
					for (int i = 0; i < 25; i++) {
						swiWaitForVBlank();
					}
				}

				if ((getFileSize(ms().dsiWarePrvPath.c_str()) == 0) && (NDSHeader.prvSavSize > 0)) {
					consoleDemoInit();
					iprintf("Creating private save file...\n");
					iprintf ("\n");
					fadeType = true;

					static const int BUFFER_SIZE = 4096;
					char buffer[BUFFER_SIZE];
					toncset(buffer, 0, sizeof(buffer));
					char savHdrPath[64];
					snprintf(savHdrPath, sizeof(savHdrPath), "nitro:/DSiWareSaveHeaders/%X.savhdr",
						 (unsigned int)NDSHeader.prvSavSize);
					FILE *hdrFile = fopen(savHdrPath, "rb");
					if (hdrFile)
						fread(buffer, 1, 0x200, hdrFile);
					fclose(hdrFile);

					FILE *pFile = fopen(ms().dsiWarePrvPath.c_str(), "wb");
					if (pFile) {
						fwrite(buffer, 1, sizeof(buffer), pFile);
						fseek(pFile, NDSHeader.prvSavSize - 1, SEEK_SET);
						fputc('\0', pFile);
						fclose(pFile);
					}
					iprintf("Private save file created!\n");

					for (int i = 0; i < 30; i++) {
						swiWaitForVBlank();
					}
					fadeType = false;
					for (int i = 0; i < 25; i++) {
						swiWaitForVBlank();
					}
				}

				if (sdFound() && !ms().previousUsedDevice) {
					argarray.push_back((char*)(useNightly ? "sd:/_nds/nds-bootstrap-nightly.nds" : "sd:/_nds/nds-bootstrap-release.nds"));
				} else {
					argarray.push_back((char*)(useNightly ? "/_nds/nds-bootstrap-nightly.nds" : "/_nds/nds-bootstrap-release.nds"));
				}

				char sfnSrl[62];
				char sfnPub[62];
				char sfnPrv[62];
				if (ms().previousUsedDevice && ms().dsiWareToSD && sdFound()) {
					if (access("sd:/_nds/TWiLightMenu/tempDSiWare.pub.bak", F_OK) == 0) {
						if (access("sd:/_nds/TWiLightMenu/tempDSiWare.pub", F_OK) == 0) {
							remove("sd:/_nds/TWiLightMenu/tempDSiWare.pub");
						}
						rename("sd:/_nds/TWiLightMenu/tempDSiWare.pub.bak", "sd:/_nds/TWiLightMenu/tempDSiWare.pub");
					}
					if (access("sd:/_nds/TWiLightMenu/tempDSiWare.prv.bak", F_OK) == 0) {
						if (access("sd:/_nds/TWiLightMenu/tempDSiWare.prv", F_OK) == 0) {
							remove("sd:/_nds/TWiLightMenu/tempDSiWare.prv");
						}
						rename("sd:/_nds/TWiLightMenu/tempDSiWare.prv.bak", "sd:/_nds/TWiLightMenu/tempDSiWare.prv");
					}
					fatGetAliasPath("sd:/", "sd:/_nds/TWiLightMenu/tempDSiWare.dsi", sfnSrl);
					fatGetAliasPath("sd:/", "sd:/_nds/TWiLightMenu/tempDSiWare.pub", sfnPub);
					fatGetAliasPath("sd:/", "sd:/_nds/TWiLightMenu/tempDSiWare.prv", sfnPrv);
				} else {
					fatGetAliasPath(ms().previousUsedDevice ? "fat:/" : "sd:/", ms().dsiWareSrlPath.c_str(), sfnSrl);
					fatGetAliasPath(ms().previousUsedDevice ? "fat:/" : "sd:/", ms().dsiWarePubPath.c_str(), sfnPub);
					fatGetAliasPath(ms().previousUsedDevice ? "fat:/" : "sd:/", ms().dsiWarePrvPath.c_str(), sfnPrv);
				}

				CIniFile bootstrapini(sdFound() ? BOOTSTRAP_INI_SD : BOOTSTRAP_INI_FC);
				bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().previousUsedDevice && ms().dsiWareToSD && sdFound() ? "sd:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath);
				bootstrapini.SetString("NDS-BOOTSTRAP", "APP_PATH", sfnSrl);
				bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", sfnPub);
				bootstrapini.SetString("NDS-BOOTSTRAP", "PRV_PATH", sfnPrv);
				bootstrapini.SetString("NDS-BOOTSTRAP", "AP_FIX_PATH", "");
				bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
				bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE",
					(perGameSettings_language == -2 ? ms().gameLanguage : perGameSettings_language));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION",
					(perGameSettings_region == -3 ? ms().gameRegion : perGameSettings_region));
				bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", true);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", true);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", true);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", 5);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "GAME_SOFT_RESET", 1);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_REGION", 0);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", 0);
				bootstrapini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", 
					(ms().forceSleepPatch
				|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !sys().isRegularDS())
				|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !sys().isRegularDS())
				|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !sys().isRegularDS())
				|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS()))
				);
				bootstrapini.SaveIniFile(sdFound() ? BOOTSTRAP_INI_SD : BOOTSTRAP_INI_FC);

				if (!isDSiMode() && (!ms().previousUsedDevice || (ms().previousUsedDevice && ms().dsiWareToSD && sdFound()))) {
					*(u32*)(0x02000000) |= BIT(3);
					*(u32*)(0x02000004) = 0;
					*(bool*)(0x02000010) = useNightly;
					*(bool*)(0x02000014) = useWidescreen;
				}

				if ((ms().consoleModel >= 2 && !useWidescreen) || ms().consoleModel < 2 || ms().macroMode) {
					remove((ms().previousUsedDevice && !ms().dsiWareToSD) ? "fat:/_nds/nds-bootstrap/wideCheatData.bin" : "sd:/_nds/nds-bootstrap/wideCheatData.bin");
				} else if ((access((ms().previousUsedDevice && !ms().dsiWareToSD) ? "fat:/_nds/nds-bootstrap/wideCheatData.bin" : "sd:/_nds/nds-bootstrap/wideCheatData.bin", F_OK) == 0) && (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0)) {
					if (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) {
						rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak");
					}
					if (rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
						tonccpy((u32 *)0x02000300, sr_data_srllastran, 0x020);
						DC_FlushAll();
						fifoSendValue32(FIFO_USER_08, 1);
						stop();
					}
				}

				if (!isDSiMode() && (!ms().previousUsedDevice || (ms().previousUsedDevice && ms().dsiWareToSD && sdFound()))) {
					ntrStartSdGame();
				}

				err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1);
			}
		} else {
			// Move .pub and/or .prv out of "saves" folder
			std::string filename = ms().romPath[ms().previousUsedDevice];
			const size_t last_slash_idx = filename.find_last_of("/");
			if (std::string::npos != last_slash_idx) {
				filename.erase(0, last_slash_idx + 1);
			}

			loadPerGameSettings(filename);

			const char *typeToReplace = ".nds";
			if (extention(filename, ".dsi")) {
				typeToReplace = ".dsi";
			} else if (extention(filename, ".ids")) {
				typeToReplace = ".ids";
			} else if (extention(filename, ".srl")) {
				typeToReplace = ".srl";
			} else if (extention(filename, ".app")) {
				typeToReplace = ".app";
			}

			std::string pubname = replaceAll(filename, typeToReplace, getPubExtension());
			std::string prvname = replaceAll(filename, typeToReplace, getPrvExtension());
			std::string pubnameUl = replaceAll(filename, typeToReplace, ".pub");
			std::string prvnameUl = replaceAll(filename, typeToReplace, ".prv");
			std::string romfolder = ms().romPath[ms().previousUsedDevice];
			while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
				romfolder.resize(romfolder.size()-1);
			}
			std::string romFolderNoSlash = romfolder;
			RemoveTrailingSlashes(romFolderNoSlash);
			std::string pubpath = romFolderNoSlash + "/saves/" + pubname;
			std::string prvpath = romFolderNoSlash + "/saves/" + prvname;
			std::string pubpathUl = romFolderNoSlash + "/" + pubnameUl;
			std::string prvpathUl = romFolderNoSlash + "/" + prvnameUl;
			if (access(pubpath.c_str(), F_OK) == 0)
			{
				rename(pubpath.c_str(), pubpathUl.c_str());
			}
			if (access(prvpath.c_str(), F_OK) == 0)
			{
				rename(prvpath.c_str(), prvpathUl.c_str());
			}

			unlaunchRomBoot(ms().previousUsedDevice ? "sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath);
		}
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::ENESDSLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/nesds.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to nesDS as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EGameYobLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/gameyob.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to GameYob as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::ES8DSLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		mkdir(ms().previousUsedDevice ? "fat:/data" : "sd:/data", 0777);
		mkdir(ms().previousUsedDevice ? "fat:/data/s8ds" : "sd:/data/s8ds", 0777);

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/S8DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to S8DS as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::ERVideoLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass video to Rocket Video Player as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EMPEG4Launch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass video to FastVideoDS as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EStellaDSLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/StellaDS.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/StellaDS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to StellaDS as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EPicoDriveTWLLaunch)
	{
		if (ms().showMd >= 2 || access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to PicoDrive TWL as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EGBANativeLaunch)
	{
		if (!sys().isRegularDS() || *(u16*)(0x020000C0) == 0 || (ms().showGba != 1) || access(ms().romPath[true].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		std::string savepath = replaceAll(ms().romPath[true], ".gba", ".sav");
		u32 romSize = getFileSize(ms().romPath[true].c_str());
		if (romSize > 0x2000000) romSize = 0x2000000;
		u32 savesize = getFileSize(savepath.c_str());
		if (savesize > 0x20000) savesize = 0x20000;

		consoleDemoInit();
		iprintf("Now Loading...\n");
		iprintf("\n");
		fadeType = true;

		u32 ptr = 0x08000000;
		char titleID[4];
		FILE* gbaFile = fopen(ms().romPath[true].c_str(), "rb");
		fseek(gbaFile, 0xAC, SEEK_SET);
		fread(&titleID, 1, 4, gbaFile);
		if (strncmp(titleID, "AGBJ", 4) == 0 && romSize <= 0x40000) {
			ptr += 0x400;
		}
		fseek(gbaFile, 0, SEEK_SET);

		extern char copyBuf[0x8000];
		bool nor = false;
		if (*(u16*)(0x020000C0) == 0x5A45 && strncmp(titleID, "AGBJ", 4) != 0) {
			cExpansion::SetRompage(0);
			expansion().SetRampage(cExpansion::ENorPage);
			cExpansion::OpenNorWrite();
			cExpansion::SetSerialMode();
			nor = true;
		} else if (*(u16*)(0x020000C0) == 0x4353 && romSize > 0x1FFFFFE) {
			romSize = 0x1FFFFFE;
		}

		if (!nor) {
			for (u32 len = romSize; len > 0; len -= 0x8000) {
				if (fread(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), gbaFile) > 0) {
					s2RamAccess(true);
					tonccpy((u16*)ptr, &copyBuf, (len>0x8000 ? 0x8000 : len));
					s2RamAccess(false);
					ptr += 0x8000;
				} else {
					break;
				}
			}
			fclose(gbaFile);
		}

		ptr = 0x0A000000;

		if (savesize > 0) {
			FILE* savFile = fopen(savepath.c_str(), "rb");
			for (u32 len = (savesize > 0x10000 ? 0x10000 : savesize); len > 0; len -= 0x8000) {
				if (fread(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), savFile) > 0) {
					gbaSramAccess(true);	// Switch to GBA SRAM
					cExpansion::WriteSram(ptr,(u8*)copyBuf,0x8000);
					gbaSramAccess(false);	// Switch out of GBA SRAM
					ptr += 0x8000;
				} else {
					break;
				}
			}
			fclose(savFile);
		}

		fadeType = false;
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}

		argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/gbapatcher.srldr";
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1);
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EA7800DSLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/A7800DS.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/A7800DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to A7800DS as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EA5200DSLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/A5200DS.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/A5200DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to A5200DS as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::ENitroGrafxLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		mkdir(ms().previousUsedDevice ? "fat:/data" : "sd:/data", 0777);
		mkdir(ms().previousUsedDevice ? "fat:/data/NitroGrafx" : "sd:/data/NitroGrafx", 0777);

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to NitroGrafx as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EXEGSDSLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/XEGS-DS.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/XEGS-DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to XEGS-DS as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::ENINTVDSLaunch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NINTV-DS.nds";
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)"fat:/_nds/TWiLightMenu/emulators/NINTV-DS.nds";
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to NINTV-DS as argument
	}
	else if (ms().launchType[ms().previousUsedDevice] == Launch::EGBARunner2Launch)
	{
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0) return;	// Skip to running TWiLight Menu++

		argarray.at(0) = (char*)(ms().gbar2DldiAccess ? "sd:/_nds/GBARunner2_arm7dldi_ds.nds" : "sd:/_nds/GBARunner2_arm9dldi_ds.nds");
		if (isDSiMode() || REG_SCFG_EXT != 0)
		{
			argarray.at(0) = (char*)(ms().consoleModel > 0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds");
		}
		if(!isDSiMode() || access(argarray[0], F_OK) != 0)
		{
			argarray.at(0) = (char*)(ms().gbar2DldiAccess ? "fat:/_nds/GBARunner2_arm7dldi_ds.nds" : "fat:/_nds/GBARunner2_arm9dldi_ds.nds");
			if (isDSiMode() || REG_SCFG_EXT != 0)
			{
				argarray.at(0) = (char*)(ms().consoleModel > 0 ? "fat:/_nds/GBARunner2_arm7dldi_3ds.nds" : "fat:/_nds/GBARunner2_arm7dldi_dsi.nds");
			}
		}
		err = runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, -1); // Pass ROM to GBARunner2 as argument
	}
	if (err > 0) {
		consoleDemoInit();
		iprintf("Start failed. Error %i\n", err);
		fadeType = true;
		for (int i = 0; i < 60*3; i++) {
			swiWaitForVBlank();
		}
		fadeType = false;
		for (int i = 0; i < 25; i++) {
			swiWaitForVBlank();
		}
	}
}

bool graphicsInited = false;
void graphicsInit(void) {
	if(graphicsInited)
		return;

	graphicsInited = true;

	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);

	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankC(VRAM_C_SUB_BG);

	bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(3, 3);
	bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(2, 2);
	bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(7, 3);
	bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 3, 0);
	bgSetPriority(6, 2);

	BG_PALETTE[0x10] = 0xFFFF;
	BG_PALETTE_SUB[0x10] = 0xFFFF;
	toncset16(bgGetGfxPtr(3), 0x1010, 256 * 192);
	toncset16(bgGetGfxPtr(7), 0x1010, 256 * 192);

	fontInit();
}

void resetSettingsPrompt(void) {
	TWLSettings settings;
	settings.loadSettings();

	langInit(settings.getGuiLanguageString().c_str());

	graphicsInit();

	fadeType = true;

	Alignment align = ms().rtl() ? Alignment::right : Alignment::left;
	int x = ms().rtl() ? 256 - 2 : 2;

	clearText();
	printLarge(false, x, 0, STR_RESET_TWILIGHT_SETTINGS, align);
	int y = calcLargeFontHeight(STR_RESET_TWILIGHT_SETTINGS);
	printSmall(false, x, y, STR_PGS_WILL_BE_KEPT, align);

	printSmall(false, x, y + 20, STR_A_YES, align);
	printSmall(false, x, y + 20 + 14, STR_B_NO, align);

	updateText(false);

	u16 pressed = 0;
	do {
		swiWaitForVBlank();
		scanKeys();
		pressed = keysDown();
	} while (!(pressed & (KEY_A | KEY_B)));

	if (pressed & KEY_A) {
		remove(settingsinipath);	// Delete "settings.ini"
	}


	fadeType = false;
	for (int i = 0; i < 30; i++)
		swiWaitForVBlank();

	clearText();
	updateText(false);
}

static bool languageNowSet = false;
static bool regionNowSet = false;

const char *languages[] = {
	"日本語",
	"English",
	"Français",
	"Deutsch",
	"Italiano",
	"Español",
	"中文 (简体)",
	"한국어",
	"中文 (繁體)",
	"Polski",
	"Português (Portugal)",
	"Русский",
	"Svenska",
	"Dansk",
	"Tiếng Việt",
	"Türkçe",
	"Українська",
	"Magyar",
	"Norsk",
	"עברית",
	"Nederlands",
	"Bahasa Indonesia",
	"Ελληνικά",
	"Български",
	"Română",
	"العربية",
	"Português (Brasil)",
};

const int guiLanguages[] = {
	TWLSettings::TLanguage::ELangIndonesian,
	TWLSettings::TLanguage::ELangDanish,
	TWLSettings::TLanguage::ELangGerman,
	TWLSettings::TLanguage::ELangEnglish,
	TWLSettings::TLanguage::ELangSpanish,
	TWLSettings::TLanguage::ELangFrench,
	TWLSettings::TLanguage::ELangItalian,
	TWLSettings::TLanguage::ELangHungarian,
	TWLSettings::TLanguage::ELangDutch,
	TWLSettings::TLanguage::ELangNorwegian,
	TWLSettings::TLanguage::ELangPolish,
	TWLSettings::TLanguage::ELangPortuguese,
	TWLSettings::TLanguage::ELangPortugueseBrazil,
	TWLSettings::TLanguage::ELangRomanian,
	TWLSettings::TLanguage::ELangSwedish,
	TWLSettings::TLanguage::ELangVietnamese,
	TWLSettings::TLanguage::ELangTurkish,
	TWLSettings::TLanguage::ELangGreek,
	TWLSettings::TLanguage::ELangBulgarian,
	TWLSettings::TLanguage::ELangRussian,
	TWLSettings::TLanguage::ELangUkrainian,
	TWLSettings::TLanguage::ELangHebrew,
	TWLSettings::TLanguage::ELangArabic,
	TWLSettings::TLanguage::ELangChineseS,
	TWLSettings::TLanguage::ELangChineseT,
	TWLSettings::TLanguage::ELangJapanese,
	TWLSettings::TLanguage::ELangKorean,
};

const int gameLanguages[] = {
	TWLSettings::TLanguage::ELangGerman,
	TWLSettings::TLanguage::ELangEnglish,
	TWLSettings::TLanguage::ELangSpanish,
	TWLSettings::TLanguage::ELangFrench,
	TWLSettings::TLanguage::ELangItalian,
	TWLSettings::TLanguage::ELangChineseS,
	TWLSettings::TLanguage::ELangJapanese,
	TWLSettings::TLanguage::ELangKorean
};
const int titleLanguages[] = {
	TWLSettings::TLanguage::ELangGerman,
	TWLSettings::TLanguage::ELangEnglish,
	TWLSettings::TLanguage::ELangSpanish,
	TWLSettings::TLanguage::ELangFrench,
	TWLSettings::TLanguage::ELangItalian,
	TWLSettings::TLanguage::ELangJapanese
};

static const char* displayLanguage(int l, int type) {
	if(l == -1) {
		return STR_SYSTEM.c_str();
	} else {
		switch(type) {
			case 0:
			default:
				return languages[guiLanguages[l]];
			case 1:
				return languages[gameLanguages[l]];
			case 2:
				return languages[titleLanguages[l]];
		}
	}
}

void languageSelect(void) {
	graphicsInit();
	langInit();

	fadeType = true;

	int guiLanguage = -1, gameLanguage = -1, titleLanguage = -1;
	for(uint i = 0; i < sizeof(guiLanguages) / sizeof(guiLanguages[0]); i++) {
		if(guiLanguages[i] == ms().guiLanguage)
			guiLanguage = i;
	}
	for(uint i = 0; i < sizeof(gameLanguages) / sizeof(gameLanguages[0]); i++) {
		if(gameLanguages[i] == ms().gameLanguage)
			gameLanguage = i;
	}
	for(uint i = 0; i < sizeof(titleLanguages) / sizeof(titleLanguages[0]); i++) {
		if(titleLanguages[i] == ms().titleLanguage)
			titleLanguage = i;
	}

	printSmall(true, 2, 4, "\uE07Eを使用して言語を選択してください。");
	printSmall(true, 2, 28, "Select your language with \uE07E.");
	printSmall(true, 2, 52, "Sélectionnez votre langage avec \uE07E.");
	printSmall(true, 2, 76, "Wähle deine Sprache mit \uE07E.");
	printSmall(true, 2, 100, "Seleziona la tua lingua con \uE07E.");
	printSmall(true, 2, 124, "Selecciona tu idioma con \uE07E.");
	printSmall(true, 2, 148, "使用 \uE07E 选择你的语言。");
	printSmall(true, 2, 172, "\uE07E를 사용하여 언어를 선택해주세요.");
	updateText(true);

	int cursorPosition = 0;
	char buffer[64] = {0};
	u16 held = 0, pressed = 0;
	while (1) {
		clearText(false);

		Alignment align = ms().rtl() ? Alignment::right : Alignment::left;
		int x1 = ms().rtl() ? 256 - 2 : 2, x2 = ms().rtl() ? 256 - 15 : 15;

		printLarge(false, x1, 0, STR_SELECT_YOUR_LANGUAGE, align);

		snprintf(buffer, sizeof(buffer), STR_GUI.c_str(), displayLanguage(guiLanguage, 0));
		printSmall(false, x2, 20, buffer, align);
		snprintf(buffer, sizeof(buffer), STR_GAME.c_str(), displayLanguage(gameLanguage, 1));
		printSmall(false, x2, 34, buffer, align);
		snprintf(buffer, sizeof(buffer), STR_DS_BANNER_TITLE.c_str(), displayLanguage(titleLanguage, 2));
		printSmall(false, x2, 48, buffer, align);

		printSmall(false, x1, 20 + cursorPosition * 14, ms().rtl() ? "<" : ">", align);

		printSmall(false, x1, 68, STR_UP_DOWN_CHOOSE, align);
		printSmall(false, x1, 82, STR_LEFT_RIGHT_CHANGE_LANGUAGE, align);
		printSmall(false, x1, 96, STR_A_PROCEED, align);

		updateText(false);

		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if (held & KEY_UP) {
			if (cursorPosition > 0)
				cursorPosition--;
		} else if (held & KEY_DOWN) {
			if (cursorPosition < 2)
				cursorPosition++;
		} else if (held & KEY_LEFT) {
			switch (cursorPosition) {
				case 0:
					if (guiLanguage > -1) {
						guiLanguage--;
						ms().guiLanguage = guiLanguage == -1 ? guiLanguage : guiLanguages[guiLanguage];
						langInit();
					}
					break;
				case 1:
					if (gameLanguage > -1)
						gameLanguage--;
					break;
				case 2:
					if (titleLanguage > -1)
						titleLanguage--;
					break;
			}
		} else if (held & KEY_RIGHT) {
			switch (cursorPosition) {
				case 0:
					if (guiLanguage < (int)(sizeof(languages) / sizeof(languages[0])) - 1) {
						guiLanguage++;
						ms().guiLanguage = guiLanguage == -1 ? guiLanguage : guiLanguages[guiLanguage];
						langInit();
					}
					break;
				case 1:
					if (gameLanguage < 7)
						gameLanguage++;
					break;
				case 2:
					if (titleLanguage < 5)
						titleLanguage++;
					break;
			}
		}

		if (pressed & KEY_A) {
			ms().gameLanguage = gameLanguage == -1 ? gameLanguage : gameLanguages[gameLanguage];
			ms().titleLanguage = titleLanguage == -1 ? titleLanguage : titleLanguages[titleLanguage];
			ms().languageSet = true;
			languageNowSet = true;
			break;
		}
	}

	fadeType = false;
	for (int i = 0; i < 30; i++)
		swiWaitForVBlank();

	clearText();
	updateText(true);
	updateText(false);
}

const std::string *regions[] {
	&STR_GAME_SPECIFIC,
	&STR_SYSTEM,
	&STR_JAPAN,
	&STR_USA,
	&STR_EUROPE,
	&STR_AUSTRALIA,
	&STR_CHINA,
	&STR_KOREA
};

void regionSelect(void) {
	graphicsInit();
	langInit();

	fadeType = true;

	Alignment align = ms().rtl() ? Alignment::right : Alignment::left;
	int x1 = ms().rtl() ? 256 - 2 : 2, x2 = ms().rtl() ? 256 - 15 : 15;

	u16 held = 0, pressed = 0;
	while (1) {
		clearText();
		printLarge(false, x1, 0, STR_SELECT_YOUR_REGION, align);

		for(uint i = dsiFeatures() ? 0 : 2, p = 0; i < sizeof(regions) / sizeof(regions[0]); i++, p++) {
			printSmall(false, x2, 20 + p * 14, *regions[i], align);
		}

		printSmall(false, x1, 20 + (ms().gameRegion + (dsiFeatures() ? 2 : 0)) * 14, ms().rtl() ? "<" : ">", align);

		int y = 20 + (sizeof(regions) / sizeof(regions[0])) * 14 + 6 - (dsiFeatures() ? 0 : 28);
		printSmall(false, x1, y, STR_UP_DOWN_CHOOSE, align);
		printSmall(false, x1, y + 14, STR_A_PROCEED, align);

		updateText(false);

		do {
			swiWaitForVBlank();
			scanKeys();
			pressed = keysDown();
			held = keysDownRepeat();
		} while(!held);

		if (held & KEY_UP) {
			if (ms().gameRegion > (dsiFeatures() ? -2 : 0))
				ms().gameRegion--;
		} else if (held & KEY_DOWN) {
			if (ms().gameRegion < (int)(sizeof(regions) / sizeof(regions[0])) - 3)
				ms().gameRegion++;
		}

		if (pressed & KEY_A) {
			ms().regionSet = true;
			regionNowSet = true;
			break;
		}
	}


	fadeType = false;
	for (int i = 0; i < 30; i++)
		swiWaitForVBlank();

	clearText();
	updateText(false);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	/*extern char *fake_heap_end;
	*fake_heap_end = 0;*/

	defaultExceptionHandler();
	sys().initFilesystem("/_nds/TWiLightMenu/main.srldr");
	ms();

	if (!sys().fatInitOk())
	{
		consoleDemoInit();
		iprintf("FAT init failed!");
		stop();
	}

	keysSetRepeat(25, 5);

	*(u32*)0x02FFFDFC = 0; // Reset TWLCFG location

	u32 softResetParamsBak = 0;
	if (*(u32*)(0x02000004) != 0) {
		*(u32*)(0x02000000) = 0; // Clear soft-reset params
	} else {
		softResetParamsBak = *(u32*)(0x02000000);
	}
	// Soft-reset parameters
	/*
		0: Skip DS(i) splash
		1: Skip TWLMenu++ splash
		2: Skip last-run game
		3: Skip flashcard ROM check (Used for launching DSiWare from flashcards booted in DS mode)
	*/

	if (sys().isRegularDS()) {
		sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM
		if (*(u16*)(0x020000C0) != 0x334D && *(u16*)(0x020000C0) != 0x3647 && *(u16*)(0x020000C0) != 0x4353 && *(u16*)(0x020000C0) != 0x5A45) {
			*(u16*)(0x020000C0) = 0;	// Clear Slot-2 flashcard flag
		}

		if (*(u16*)(0x020000C0) == 0) {
		  if (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS) {
			*(vu16*)(0x08000000) = 0x4D54;	// Write test
			if (*(vu16*)(0x08000000) != 0x4D54) {	// If not writeable
				_M3_changeMode(M3_MODE_RAM);	// Try again with M3
				*(u16*)(0x020000C0) = 0x334D;
				*(vu16*)(0x08000000) = 0x4D54;
			}
			if (*(vu16*)(0x08000000) != 0x4D54) {
				_G6_SelectOperation(G6_MODE_RAM);	// Try again with G6
				*(u16*)(0x020000C0) = 0x3647;
				*(vu16*)(0x08000000) = 0x4D54;
			}
			if (*(vu16*)(0x08000000) != 0x4D54) {
				_SC_changeMode(SC_MODE_RAM);	// Try again with SuperCard
				*(u16*)(0x020000C0) = 0x4353;
				*(vu16*)(0x08000000) = 0x4D54;
			}
			if (*(vu16*)(0x08000000) != 0x4D54) {
				cExpansion::SetRompage(381);	// Try again with EZ Flash
				cExpansion::OpenNorWrite();
				cExpansion::SetSerialMode();
				*(u16*)(0x020000C0) = 0x5A45;
				*(vu16*)(0x08000000) = 0x4D54;
			}
			if (*(vu16*)(0x08000000) != 0x4D54) {
				*(u16*)(0x020000C0) = 0;
			}
		  } else if (io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) {
			if (memcmp(io_dldi_data->friendlyName, "M3 Adapter", 10) == 0) {
				*(u16*)(0x020000C0) = 0x334D;
			} else if (memcmp(io_dldi_data->friendlyName, "G6", 2) == 0) {
				*(u16*)(0x020000C0) = 0x3647;
			} else if (memcmp(io_dldi_data->friendlyName, "SuperCard", 9) == 0) {
				*(u16*)(0x020000C0) = 0x4353;
			}
		  }
		}
	}

	bool is3DS = fifoGetValue32(FIFO_USER_05) != 0xD2;

	useTwlCfg = (REG_SCFG_EXT!=0 && (*(u8*)0x02000400 != 0) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));
	if (REG_SCFG_EXT != 0) {
		bool nandMounted = false;
		if (!useTwlCfg && isDSiMode() && sdFound() && sys().arm7SCFGLocked() && !is3DS) {
			nandMounted = fatMountSimple("nand", &io_dsi_nand);
			if (nandMounted) {
				//toncset((void*)0x02000004, 0, 0x3FFC); // Already done by exploit

				FILE* twlCfgFile = fopen("nand:/shared1/TWLCFG0.dat", "rb");
				fseek(twlCfgFile, 0x88, SEEK_SET);
				fread((void*)0x02000400, 1, 0x128, twlCfgFile);
				fclose(twlCfgFile);

				// WiFi RAM data
				u8* twlCfg = (u8*)0x02000400;
				readFirmware(0x1FD, twlCfg+0x1E0, 1); // WlFirm Type (1=DWM-W015, 2=W024, 3=W028)
				if (twlCfg[0x1E0] == 2 || twlCfg[0x1E0] == 3) {
					toncset32(twlCfg+0x1E4, 0x520000, 1); // WlFirm RAM vars
					toncset32(twlCfg+0x1E8, 0x520000, 1); // WlFirm RAM base
					toncset32(twlCfg+0x1EC, 0x020000, 1); // WlFirm RAM size
				} else {
					toncset32(twlCfg+0x1E4, 0x500400, 1); // WlFirm RAM vars
					toncset32(twlCfg+0x1E8, 0x500000, 1); // WlFirm RAM base
					toncset32(twlCfg+0x1EC, 0x02E000, 1); // WlFirm RAM size
				}
				*(u16*)(twlCfg+0x1E2) = swiCRC16(0xFFFF, twlCfg+0x1E4, 0xC); // WlFirm CRC16

				twlCfgFile = fopen("nand:/sys/HWINFO_N.dat", "rb");
				fseek(twlCfgFile, 0x88, SEEK_SET);
				fread((void*)0x02000600, 1, 0x14, twlCfgFile);
				fclose(twlCfgFile);

				useTwlCfg = true;
				tonccpy((void*)0x0377C000, (void*)0x02000000, 0x4000);
				*(vu32*)(0x0377C000) = BIT(0);
			}
		} else {
			if (useTwlCfg) {
				tonccpy((void*)0x0377C000, (void*)0x02000000, 0x4000);
				*(vu32*)(0x0377C000) = BIT(0);
			} else {
				tonccpy((void*)0x02000000, (void*)0x0377C000, 0x4000); // Restore from DSi WRAM
				useTwlCfg = ((*(u8*)0x02000400 != 0) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));
				if (*(vu32*)(0x0377C000) & BIT(0)) {
					softResetParamsBak |= BIT(0);
				}
			}
		}
		if (useTwlCfg && isDSiMode() && sdFound() && sys().arm7SCFGLocked() && *(u32*)0x020007F0 != 0x4D44544C) {
			u32 srBackendId[2] = {*(u32*)0x02000428, *(u32*)0x0200042C};
			u16 tidPart = 0;
			tonccpy(&tidPart, (void*)((u32)srBackendId+6), sizeof(u16));
			if (is3DS) {
				if (tidPart != 0x0003) {
					u32 currentSrBackendId[2] = {0};
					u8 sysValue = 0;

					TWLSettings settings;
					settings.loadSettings();

					switch (settings.dsiWareExploit) {
						case 1:
						default:
							currentSrBackendId[0] = 0x4B344441;		// SUDOKU
							break;
						case 2:
							currentSrBackendId[0] = 0x4B513941;		// Legend of Zelda: Four Swords
							break;
						case 3:
							currentSrBackendId[0] = 0x4B464441;		// Fieldrunners
							break;
						case 4:
							currentSrBackendId[0] = 0x4B475241;		// Guitar Rock Tour
							break;
						case 5:
							currentSrBackendId[0] = 0x4B475541;		// Flipnote Studio
							break;
						case 6:
							currentSrBackendId[0] = 0x4B554E41;		// UNO
							break;
						case 7:
							currentSrBackendId[0] = 0x484E4941;		// Nintendo DSi Camera
							break;
					}
					switch (settings.sysRegion) {
						case 0:
							sysValue = 0x4A;		// JPN
							break;
						case 1:
							sysValue = 0x45;		// USA
							break;
						case 2:
							sysValue = (settings.dsiWareExploit==7 ? 0x50 : 0x56);		// EUR
							break;
						case 3:
							sysValue = (settings.dsiWareExploit==7 ? 0x55 : 0x56);		// AUS
							break;
						case 4:
							sysValue = 0x43;		// CHN
							break;
						case 5:
							sysValue = 0x4B;		// KOR
							break;
					}
					tonccpy(&currentSrBackendId, &sysValue, 1);
					currentSrBackendId[1] = (settings.dsiWareExploit==7 ? 0x00030005 : 0x00030004);

					if (settings.dsiWareExploit > 0) {
						mkdir("sd:/_nds/nds-bootstrap", 0777);
						FILE* file = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "wb");
						if (file) {
							fwrite(currentSrBackendId, sizeof(u32), 2, file);
						}
						fclose(file);
					} else if (access("sd:/_nds/nds-bootstrap/srBackendId.bin", F_OK) == 0) {
						remove("sd:/_nds/nds-bootstrap/srBackendId.bin");
					}
				} else {
					mkdir("sd:/_nds/nds-bootstrap", 0777);
					FILE* srBackendIdFile = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "wb");
					fwrite(srBackendId, 1, 8, srBackendIdFile);
					fclose(srBackendIdFile);
				}
			} else {
				if (tidPart != 0x0003) {
					if (!nandMounted) {
						nandMounted = fatMountSimple("nand", &io_dsi_nand);
					}
					// Read correct title ID of launched System Menu title
					FILE* twlCfgFile = fopen("nand:/shared1/TWLCFG0.dat", "rb");
					fseek(twlCfgFile, 0xB0, SEEK_SET);
					fread(srBackendId, sizeof(u32), 2, twlCfgFile);
					fclose(twlCfgFile);

					tonccpy(&tidPart, (void*)((u32)srBackendId+6), sizeof(u16));
				}
				if (tidPart == 0x0003) {
					mkdir("sd:/_nds/nds-bootstrap", 0777);
					FILE* srBackendIdFile = fopen("sd:/_nds/nds-bootstrap/srBackendId.bin", "wb");
					fwrite(srBackendId, 1, 8, srBackendIdFile);
					fclose(srBackendIdFile);
				} else if (access("sd:/_nds/nds-bootstrap/srBackendId.bin", F_OK) == 0) {
					remove("sd:/_nds/nds-bootstrap/srBackendId.bin");
				}
			}
		} else if (access("sd:/_nds/nds-bootstrap/srBackendId.bin", F_OK) == 0) {
			remove("sd:/_nds/nds-bootstrap/srBackendId.bin");
		}
	}

	*(u32*)(0x02000000) = softResetParamsBak;

	if (access(settingsinipath, F_OK) != 0 && (access("fat:/", F_OK) == 0)) {
		settingsinipath =
		    DSIMENUPP_INI_FC; // Fallback to .ini path on flashcard, if not found on
							   // SD card, or if SD access is disabled
	}

	scanKeys();
	if ((keysHeld() & KEY_A)
	 && (keysHeld() & KEY_B)
	 && (keysHeld() & KEY_X)
	 && (keysHeld() & KEY_Y))
	{
		runGraphicIrq();
		resetSettingsPrompt();
	}

	ms().loadSettings();
	bs().loadSettings();

	if (!ms().languageSet) {
		languageSelect();
	}

	if (!ms().regionSet || (!dsiFeatures() && ms().gameRegion < 0)) {
		if (!dsiFeatures() && ms().gameRegion < 0) ms().gameRegion = 0;
		runGraphicIrq();
		regionSelect();
	}

	if (isDSiMode()) {
		scanKeys();
		if (!(keysHeld() & KEY_SELECT)) {
			flashcardInit();
		}
	}

	bool fcFound = flashcardFound();

	if (ms().launchType[true] == Launch::EDSiWareLaunch) {
		// Move .pub and/or .prv back to "saves" folder
		std::string filename = ms().romPath[ms().previousUsedDevice];
		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx) {
			filename.erase(0, last_slash_idx + 1);
		}

		loadPerGameSettings(filename);

		const char *typeToReplace = ".nds";
		if (extention(filename, ".dsi")) {
			typeToReplace = ".dsi";
		} else if (extention(filename, ".ids")) {
			typeToReplace = ".ids";
		} else if (extention(filename, ".srl")) {
			typeToReplace = ".srl";
		} else if (extention(filename, ".app")) {
			typeToReplace = ".app";
		}

		std::string pubname = replaceAll(filename, typeToReplace, getPubExtension());
		std::string prvname = replaceAll(filename, typeToReplace, getPrvExtension());
		std::string pubnameUl = replaceAll(filename, typeToReplace, ".pub");
		std::string prvnameUl = replaceAll(filename, typeToReplace, ".prv");
		std::string romfolder = ms().romPath[ms().previousUsedDevice];
		while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
			romfolder.resize(romfolder.size()-1);
		}
		std::string romFolderNoSlash = romfolder;
		RemoveTrailingSlashes(romFolderNoSlash);
		std::string pubpath = romFolderNoSlash + "/saves/" + pubname;
		std::string prvpath = romFolderNoSlash + "/saves/" + prvname;
		std::string pubpathUl = romFolderNoSlash + "/" + pubnameUl;
		std::string prvpathUl = romFolderNoSlash + "/" + prvnameUl;
		if (access(pubpathUl.c_str(), F_OK) == 0)
		{
			rename(pubpathUl.c_str(), pubpath.c_str());
		}
		if (access(prvpathUl.c_str(), F_OK) == 0)
		{
			rename(prvpathUl.c_str(), prvpath.c_str());
		}
	}

	if (fcFound) {
	  if (ms().launchType[true] == Launch::ESDFlashcardLaunch) {
		// Move .sav back to "saves" folder
		std::string filename = ms().romPath[true];
		const size_t last_slash_idx = filename.find_last_of("/");
		if (std::string::npos != last_slash_idx) {
			filename.erase(0, last_slash_idx + 1);
		}

		loadPerGameSettings(filename);

		const char *typeToReplace = ".nds";
		if (extention(filename, ".dsi")) {
			typeToReplace = ".dsi";
		} else if (extention(filename, ".ids")) {
			typeToReplace = ".ids";
		} else if (extention(filename, ".srl")) {
			typeToReplace = ".srl";
		} else if (extention(filename, ".app")) {
			typeToReplace = ".app";
		}

		std::string savename = replaceAll(filename, typeToReplace, getSavExtension());
		std::string savenameFc = replaceAll(filename, typeToReplace, ".sav");
		std::string romfolder = ms().romPath[true];
		while (!romfolder.empty() && romfolder[romfolder.size()-1] != '/') {
			romfolder.resize(romfolder.size()-1);
		}
		std::string romFolderNoSlash = romfolder;
		RemoveTrailingSlashes(romFolderNoSlash);
		std::string savepath = romFolderNoSlash + "/saves/" + savename;
		std::string savepathFc = romFolderNoSlash + "/" + savenameFc;
		if (access(savepathFc.c_str(), F_OK) == 0
		&& (extention(filename, ".nds")
		 || extention(filename, ".dsi")
		 || extention(filename, ".ids")
		 || extention(filename, ".srl")
		 || extention(filename, ".app")))
		{
			rename(savepathFc.c_str(), savepath.c_str());
		}
	  } else if (sys().isRegularDS() && (*(u16*)(0x020000C0) != 0) && (ms().launchType[true] == Launch::EGBANativeLaunch)) {
			u8 byteBak = *(vu8*)(0x0A000000);
			*(vu8*)(0x0A000000) = 'T';	// SRAM write test
		  if (*(vu8*)(0x0A000000) == 'T') {	// Check if SRAM is writeable
			*(vu8*)(0x0A000000) = byteBak;
			std::string savepath = replaceAll(ms().romPath[true], ".gba", ".sav");
			u32 savesize = getFileSize(savepath.c_str());
			if (savesize > 0x20000) savesize = 0x20000;
			if (savesize > 0) {
				// Try to restore save from SRAM
				bool restoreSave = false;
				extern char copyBuf[0x8000];
				u32 ptr = 0x0A000000;
				u32 len = savesize;
				gbaSramAccess(true);	// Switch to GBA SRAM
				for (u32 i = 0; i < savesize; i += 0x8000) {
					if (ptr >= 0x0A010000 || len <= 0) {
						break;
					}
					cExpansion::ReadSram(ptr,(u8*)copyBuf,(len>0x8000 ? 0x8000 : len));
					for (u32 i2 = 0; i2 < (len>0x8000 ? 0x8000 : len); i2++) {
						if (copyBuf[i2] != 0) {
							restoreSave = true;
							break;
						}
					}
					ptr += 0x8000;
					len -= 0x8000;
					if (restoreSave) break;
				}
				gbaSramAccess(false);	// Switch out of GBA SRAM
				if (restoreSave) {
					ptr = 0x0A000000;
					len = savesize;
					FILE* savFile = fopen(savepath.c_str(), "wb");
					for (u32 i = 0; i < savesize; i += 0x8000) {
						if (ptr >= 0x0A020000 || len <= 0) {
							break;	// In case if this writes a save bigger than 128KB
						} else if (ptr >= 0x0A010000) {
							toncset(&copyBuf, 0, 0x8000);
						} else {
							gbaSramAccess(true);	// Switch to GBA SRAM
							cExpansion::ReadSram(ptr,(u8*)copyBuf,0x8000);
							gbaSramAccess(false);	// Switch out of GBA SRAM
						}
						fwrite(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), savFile);
						ptr += 0x8000;
						len -= 0x8000;
					}
					fclose(savFile);

					gbaSramAccess(true);	// Switch to GBA SRAM
					// Wipe out SRAM after restoring save
					toncset(&copyBuf, 0, 0x8000);
					cExpansion::WriteSram(0x0A000000, (u8*)copyBuf, 0x8000);
					cExpansion::WriteSram(0x0A008000, (u8*)copyBuf, 0x8000);
					gbaSramAccess(false);	// Switch out of GBA SRAM
				}
			}
		  }
	  }
	}

	if (isDSiMode() && ms().consoleModel < 2) {
		if (ms().wifiLed == -1) {
			if (*(u8*)(0x02FFFD01) == 0x13) {
				ms().wifiLed = true;
			} else if (*(u8*)(0x02FFFD01) == 0 || *(u8*)(0x02FFFD01) == 0x12) {
				ms().wifiLed = false;
			}
		} else {
			*(u8*)(0x02FFFD00) = (ms().wifiLed ? 0x13 : 0);		// WiFi On/Off
		}
	}

	if (sdFound()) {
		mkdir("sd:/_gba", 0777);
		mkdir("sd:/_nds/TWiLightMenu/gamesettings", 0777);
	}
	if (flashcardFound()) {
		mkdir("fat:/_gba", 0777);
		mkdir("fat:/_nds/TWiLightMenu/gamesettings", 0777);
	}

	if (REG_SCFG_EXT != 0) {
		*(vu32*)(0x0DFFFE0C) = 0x53524C41;		// Check for 32MB of RAM
		bool isDevConsole = (*(vu32*)(0x0DFFFE0C) == 0x53524C41);
		if (isDevConsole)
		{
			// Check for Nintendo 3DS console
			int resultModel = 1+is3DS;
			if (ms().consoleModel != resultModel || bs().consoleModel != resultModel)
			{
				ms().consoleModel = resultModel;
				bs().consoleModel = resultModel;
				ms().saveSettings();
				bs().saveSettings();
			}
		}
		else
		if (ms().consoleModel != 0 || bs().consoleModel != 0)
		{
			ms().consoleModel = 0;
			bs().consoleModel = 0;
			ms().saveSettings();
			bs().saveSettings();
		}
	}

	bool softResetParamsFound = false;
	const char* softResetParamsPath = (!ms().previousUsedDevice ? "sd:/_nds/nds-bootstrap/softResetParams.bin" : "fat:/_nds/nds-bootstrap/softResetParams.bin");
	u32 softResetParams = 0;
	FILE* file = fopen(softResetParamsPath, "rb");
	if (file) {
		fread(&softResetParams, sizeof(u32), 1, file);
		softResetParamsFound = (softResetParams != 0xFFFFFFFF);
	}
	fclose(file);

	if (softResetParamsFound) {
		scanKeys();
		if (keysHeld() & KEY_X) {
			softResetParams = 0xFFFFFFFF;
			file = fopen(softResetParamsPath, "wb");
			fwrite(&softResetParams, sizeof(u32), 1, file);
			fseek(file, 0x10 - 1, SEEK_SET);
			fputc('\0', file);
			fclose(file);
			softResetParamsFound = false;
		}
	}

	if ((access(settingsinipath, F_OK) != 0)
	|| languageNowSet || regionNowSet
	|| (ms().theme < 0) || (ms().theme == 3) || (ms().theme > 6)) {
		// Create or modify "settings.ini"
		if (ms().theme == 3) {
			ms().theme = 2;
		} else if (ms().theme < 0 || ms().theme > 6) {
			ms().theme = 0;
		}
		ms().saveSettings();
	}

	if (!softResetParamsFound && ms().dsiSplash && (REG_SCFG_EXT!=0&&ms().consoleModel<2 ? fifoGetValue32(FIFO_USER_01) != 0x01 : !(*(u32*)0x02000000 & BIT(0)))) {
		runGraphicIrq();
		bootSplashInit();
		if (REG_SCFG_EXT != 0 && ms().consoleModel < 2) fifoSendValue32(FIFO_USER_01, 10);
	}
	*(u32*)0x02000000 |= BIT(0);

	if (access(BOOTSTRAP_INI, F_OK) != 0) {
		u64 driveSize = 0;
		int gbNumber = 0;
		struct statvfs st;
		if (statvfs(sdFound() ? "sd:/" : "fat:/", &st) == 0) {
			driveSize = st.f_bsize * st.f_blocks;
		}
		for (u64 i = 0; i <= driveSize; i += 0x40000000) {
			gbNumber++;	// Get GB number
		}
		bs().cacheFatTable = (gbNumber < 32);	// Enable saving FAT table cache, if SD/Flashcard is 32GB or less

		// Create "nds-bootstrap.ini"
		bs().saveSettings();
	}
	
	scanKeys();

	if (!(*(u32*)0x02000000 & BIT(2))
	&& ((softResetParamsFound && (ms().launchType[ms().previousUsedDevice] == Launch::ESDFlashcardLaunch || ms().launchType[ms().previousUsedDevice] == Launch::EDSiWareLaunch))
	|| (ms().autorun ? !(keysHeld() & KEY_B) : (keysHeld() & KEY_B))))
	{
		//unloadNds9iAsynch();
		if (isDSiMode() && sdFound() && !fcFound && !sys().arm7SCFGLocked() && ms().limitedMode > 0) {
			*(u32*)0x02FFFD0C = ms().limitedMode == 2 ? 0x4E44544C : 0x4D44544C;
			*(u32*)0x020007F0 = 0x4D44544C;
		}
		lastRunROM();
	}

	// If in DSi mode with no flashcard & SCFG access, attempt to cut slot1 power to save battery
	if (isDSiMode() && !fcFound && !sys().arm7SCFGLocked() && !ms().autostartSlot1) {
		disableSlot1();
	}

	if (!softResetParamsFound && ms().autostartSlot1 && isDSiMode() && REG_SCFG_MC != 0x11 && !fcFound && !(keysHeld() & KEY_SELECT)) {
		//unloadNds9iAsynch();
		if (ms().slot1LaunchMethod==0 || sys().arm7SCFGLocked()) {
			dsCardLaunch();
		} else if (ms().slot1LaunchMethod==2) {
			unlaunchRomBoot("cart:");
		} else {
			runNdsFile("/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, true, false, true, true, -1);
		}
	}

	// snprintf(vertext, sizeof(vertext), "Ver %d.%d.%d   ", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH); // Doesn't work :(

	if (ms().showlogo && !(*(u32*)0x02000000 & BIT(1)))
	{
		runGraphicIrq();
		loadTitleGraphics();
		fadeType = true;
		for (int i = 0; i < 15; i++)
		{
			swiWaitForVBlank();
		}
		twlMenuVideo();
		snd().fadeOutStream();
	}
	*(u32*)0x02000000 &= ~(1UL << 1);

	scanKeys();

	if (keysHeld() & KEY_SELECT)
	{
		screenmode = 1;
	}

	srand(time(NULL));

	while (1) {
		runGraphicIrq();
		if (screenmode == 1) {
			//unloadNds9iAsynch();
			fadeColor = true;
			controlTopBright = true;
			controlBottomBright = true;
			fadeType = false;
			for (int i = 0; i < 25; i++) {
				swiWaitForVBlank();
			}
			if (!isDSiMode()) {
				chdir("fat:/");
			}
			runNdsFile("/_nds/TWiLightMenu/settings.srldr", 0, NULL, true, false, false, true, true, -1);
		} else {
			if (isDSiMode() && sdFound() && !fcFound && !sys().arm7SCFGLocked() && ms().limitedMode > 0) {
				*(u32*)0x02FFFD0C = ms().limitedMode == 2 ? 0x4E44544C : ms().limitedMode == 3 ? 0x6D44544C : 0x4D44544C;
				*(u32*)0x020007F0 = 0x4D44544C;
			}
			if (ms().showMainMenu) {
				loadMainMenu();
			}
			loadROMselect();
		}
	}

	return 0;
}
