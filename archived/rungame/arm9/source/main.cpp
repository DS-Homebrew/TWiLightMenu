#include <nds.h>
#include <nds/arm9/dldi.h>
#include <stdio.h>
#include <fat.h>
#include "fat_ext.h"
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>

#include "ndsheaderbanner.h"
#include "perGameSettings.h"
#include "fileCopy.h"
#include "common/flashcard.h"
#include "common/inifile.h"
#include "common/nds_loader_arm9.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "common/nitrofs.h"
#include "defaultSettings.h"
#include "myDSiMode.h"
#include "twlFlashcard.h"

#include "twlClockExcludeMap.h"
#include "saveMap.h"
#include "ROMList.h"

#include "sr_data_srllastran.h"		 // For rebooting into the game

static const char *unlaunchAutoLoadID = "AutoLoadInfo";

bool useTwlCfg = false;
bool externalFirmsModules = false;
bool dsModeForced = false;

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

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

/**
 * Disable TWL clock speed for a specific game.
 */
bool setClockSpeed(char gameTid[]) {
	if (!ms().ignoreBlacklists) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(twlClockExcludeList)/sizeof(twlClockExcludeList[0]); i++) {
			if (memcmp(gameTid, twlClockExcludeList[i], 3) == 0) {
				// Found match	
				dsModeForced = true;
				return false;
			}
		}
	}

	return perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
}

char filePath[PATH_MAX];

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

bool extention(const std::string& filename, const char* ext) {
	if (strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext)) {
		return false;
	} else {
		return true;
	}
}

// From FontGraphic class
std::u16string utf8to16(std::string_view text) {
	std::u16string out;
	for (uint i=0;i<text.size();) {
		char16_t c;
		if (!(text[i] & 0x80)) {
			c = text[i++];
		} else if ((text[i] & 0xE0) == 0xC0) {
			c  = (text[i++] & 0x1F) << 6;
			c |=  text[i++] & 0x3F;
		} else if ((text[i] & 0xF0) == 0xE0) {
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

void unlaunchBootDSiWare(void) {
	std::u16string path(ms().previousUsedDevice ? u"sdmc:/_nds/TWiLightMenu/tempDSiWare.dsi" : utf8to16(ms().dsiWareSrlPath));
	if (path.substr(0, 3) == u"sd:") {
		path = u"sdmc:" + path.substr(3);
	}

	tonccpy((u8*)0x02000800, unlaunchAutoLoadID, 12);
	*(u16*)(0x0200080C) = 0x3F0;			// Unlaunch Length for CRC16 (fixed, must be 3F0h)
	*(u16*)(0x0200080E) = 0;			// Unlaunch CRC16 (empty)
	*(u32*)(0x02000810) |= BIT(0);			// Load the title at 2000838h
	*(u32*)(0x02000810) |= BIT(1);			// Use colors 2000814h
	*(u16*)(0x02000814) = 0x7FFF;			// Unlaunch Upper screen BG color (0..7FFFh)
	*(u16*)(0x02000816) = 0x7FFF;			// Unlaunch Lower screen BG color (0..7FFFh)
	toncset((u8*)0x02000818, 0, 0x20+0x208+0x1C0);	// Unlaunch Reserved (zero)
	for (uint i = 0; i < std::min(path.length(), 0x103u); i++) {
		((char16_t*)0x02000838)[i] = path[i];		// Unlaunch Device:/Path/Filename.ext (16bit Unicode,end by 0000h)
	}
	*(u16*)(0x0200080E) = swiCRC16(0xFFFF, (void*)0x02000810, 0x3F0);		// Unlaunch CRC16

	fifoSendValue32(FIFO_USER_08, 1);	// Reboot
	for (int i = 0; i < 15; i++) swiWaitForVBlank();
}

bool twlBgCxiFound = false;

void wideCheck(bool useWidescreen) {
	if (ms().consoleModel < 2) return;

	CIniFile lumaConfig("sd:/luma/config.ini");

	bool wideCheatFound = (access("sd:/_nds/nds-bootstrap/wideCheatData.bin", F_OK) == 0 || access("fat:/_nds/nds-bootstrap/wideCheatData.bin", F_OK) == 0);
	if (useWidescreen && wideCheatFound && (lumaConfig.GetInt("boot", "enable_external_firm_and_modules", 0) == true)) {
		if (access("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", F_OK) == 0) {
			// If title previously launched in widescreen, move Widescreen.cxi again, and reboot again
			if (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0) {
				rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak");
			}
			if (rename("sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi", "sd:/luma/sysmodules/TwlBg.cxi") == 0) {
				tonccpy((u32*)0x02000300, sr_data_srllastran, 0x020);
				DC_FlushAll();
				fifoSendValue32(FIFO_USER_08, 1);
				stop();
			}
		} else if (twlBgCxiFound) {
			// Revert back to 4:3 for when returning to TWLMenu++
			mkdir("sd:/_nds/TWiLightMenu/TwlBg", 0777);
			if (rename("sd:/luma/sysmodules/TwlBg.cxi", "sd:/_nds/TWiLightMenu/TwlBg/Widescreen.cxi") != 0) {
				consoleDemoInit();
				iprintf("Failed to rename TwlBg.cxi\n");
				iprintf("back to Widescreen.cxi\n");
				for (int i = 0; i < 60*3; i++) swiWaitForVBlank();
			}
			if (access("sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak", F_OK) == 0) {
				rename("sd:/_nds/TWiLightMenu/TwlBg/TwlBg.cxi.bak", "sd:/luma/sysmodules/TwlBg.cxi");
			}
		}
	}
}

int lastRunROM() {
	//iprintf("Loading settings...\n");
	ms().loadSettings();

	if (ms().consoleModel < 2) {
		*(u8*)(0x02FFFD00) = (ms().wifiLed ? 0x13 : 0x12);		// WiFi On/Off
		*(u8*)(0x02FFFD02) = (ms().powerLedColor ? 0xFF : 0x00);
	}

	if (ms().macroMode) {
		powerOff(PM_BACKLIGHT_TOP);
	}

	if (ms().consoleModel >= 2) {
		twlBgCxiFound = (access("sd:/luma/sysmodules/TwlBg.cxi", F_OK) == 0);
		/*if (twlBgCxiFound) {
			iprintf("TwlBg.cxi found!\n");
		}*/
	}

	std::vector<char*> argarray;
	if (ms().launchType[ms().previousUsedDevice] == 21 && !ms().previousUsedDevice) {
		argarray.push_back(strdup("null"));
		std::string homebrewArgFat = ReplaceAll(ms().homebrewArg[ms().previousUsedDevice], "sd:", "fat:");
		argarray.push_back((char*)homebrewArgFat.c_str());
	} else if (ms().launchType[ms().previousUsedDevice] > 3) {
		argarray.push_back(strdup("null"));
		argarray.push_back(strdup(ms().homebrewArg[ms().previousUsedDevice].c_str()));
	}

	if (!(*(u32*)(0x02000000) & BIT(3))) {
		if (access(ms().romPath[ms().previousUsedDevice].c_str(), F_OK) != 0 || ms().launchType[ms().previousUsedDevice] == 0) {
			return runNdsFile ("sd:/_nds/TWiLightMenu/main.srldr", 0, NULL, true, false, false, true, true, false, -1);	// Skip to running TWiLight Menu++
		}

		if (ms().slot1Launched) {
			wideCheck(ms().wideScreen);
			return runNdsFile ("sd:/_nds/TWiLightMenu/slot1launch.srldr", 0, NULL, true, false, false, true, true, false, -1);
		}
	}

	switch (ms().launchType[ms().previousUsedDevice]) {
		//iprintf("switch (ms().launchType[])\n");
		case TWLSettings::ESDFlashcardLaunch: {
			//iprintf("TWLSettings::ESDFlashcardLaunch\n");
			ms().romfolder[ms().previousUsedDevice] = ms().romPath[ms().previousUsedDevice];
			while (!ms().romfolder[ms().previousUsedDevice].empty() && ms().romfolder[ms().previousUsedDevice][ms().romfolder[ms().previousUsedDevice].size()-1] != '/') {
				ms().romfolder[ms().previousUsedDevice].resize(ms().romfolder[ms().previousUsedDevice].size()-1);
			}
			chdir(ms().romfolder[ms().previousUsedDevice].c_str());

			std::string filename = ms().romPath[ms().previousUsedDevice];
			const size_t last_slash_idx = filename.find_last_of("/");
			if (std::string::npos != last_slash_idx) {
				filename.erase(0, last_slash_idx + 1);
			}

			loadPerGameSettings(filename);

			char game_TID[5];
			u8 unitCode = 0;

			FILE *f_nds_file = fopen(filename.c_str(), "rb");

			fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
			fread(game_TID, 1, 4, f_nds_file);
			fseek(f_nds_file, 0x12, SEEK_SET);
			fread(&unitCode, 1, 1, f_nds_file);
			game_TID[4] = 0;

			fclose(f_nds_file);

			if (((perGameSettings_useBootstrap == -1 ? ms().useBootstrap : perGameSettings_useBootstrap) && !ms().homebrewBootstrap) || !ms().previousUsedDevice || (unitCode > 0 && (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode))) {
				std::string savepath;

				bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);
				bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

				wideCheck(useWidescreen);

				if (!ms().homebrewBootstrap) {
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

					std::string savename = ReplaceAll(filename, typeToReplace, getSavExtension());
					std::string romFolderNoSlash = ms().romfolder[ms().previousUsedDevice];
					RemoveTrailingSlashes(romFolderNoSlash);
					mkdir ("saves", 0777);
					savepath = romFolderNoSlash+"/saves/"+savename;
					if (ms().previousUsedDevice && ms().fcSaveOnSd) {
						savepath = ReplaceAll(savepath, "fat:/", "sd:/");
					}

					// Create or expand save if game isn't homebrew
					u32 orgsavesize = getFileSize(savepath.c_str());
					u32 savesize = 524288;	// 512KB (default size)

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
					}
				}

				char ndsToBoot[256];
				sprintf(ndsToBoot, "sd:/_nds/nds-bootstrap-%s%s.nds", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
				if (access(ndsToBoot, F_OK) != 0) {
					sprintf(ndsToBoot, "fat:/_nds/nds-bootstrap-%s%s.nds", ms().homebrewBootstrap ? "hb-" : "", useNightly ? "nightly" : "release");
				}

				argarray.push_back(ndsToBoot);
				if (ms().previousUsedDevice || !ms().homebrewBootstrap) {
					bool boostCpu = setClockSpeed(game_TID);

					const char *bootstrapinipath = BOOTSTRAP_INI;
					CIniFile bootstrapini(bootstrapinipath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().romPath[ms().previousUsedDevice]);
					bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
					// bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
					bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", perGameSettings_language == -2 ? ms().getGameLanguage() : perGameSettings_language);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", dsModeForced ? 0 : (perGameSettings_dsiMode == -1 ? DEFAULT_DSI_MODE : perGameSettings_dsiMode));
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
					bootstrapini.SetInt( "NDS-BOOTSTRAP", "BOOST_VRAM", perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram);
					bootstrapini.SaveIniFile(bootstrapinipath);
				}

				//iprintf("Starting nds-bootstrap...\n");
				return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], (ms().homebrewBootstrap ? false : true), true, false, true, true, false, -1);
			} else {
				bool runNds_boostCpu = perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
				bool runNds_boostVram = perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram;

				std::string path;
				if ((memcmp(io_dldi_data->friendlyName, "R4(DS) - Revolution for DS", 26) == 0)
				 || (memcmp(io_dldi_data->friendlyName, "R4TF", 4) == 0)
				 || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
				 || (memcmp(io_dldi_data->friendlyName, "R4iTT", 5) == 0)
				 || (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 0xB) == 0)
    			 || (memcmp(io_dldi_data->friendlyName, "Ace3DS+", 7) == 0)) {
					CIniFile fcrompathini("fat:/_wfwd/lastsave.ini");
					path = ReplaceAll(ms().romPath[1], "fat:/", woodfat);
					fcrompathini.SetString("Save Info", "lastLoaded", path);
					fcrompathini.SaveIniFile("fat:/_wfwd/lastsave.ini");
					return runNdsFile("fat:/Wfwd.dat", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
				} else if (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0) {
					CIniFile fcrompathini("fat:/_dstwo/autoboot.ini");
					path = ReplaceAll(ms().romPath[1], "fat:/", dstwofat);
					fcrompathini.SetString("Dir Info", "fullName", path);
					fcrompathini.SaveIniFile("fat:/_dstwo/autoboot.ini");
					return runNdsFile("fat:/_dstwo/autoboot.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
				} else if ((memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0)
						 || (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0)
						 || (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0)
						 || (memcmp(io_dldi_data->friendlyName, "DSONE", 5) == 0)
						 || (memcmp(io_dldi_data->friendlyName, "M3DS", 4) == 0)
						 || (memcmp(io_dldi_data->friendlyName, "M3-DS", 5) == 0)) {
					CIniFile fcrompathini("fat:/TTMenu/YSMenu.ini");
					path = ReplaceAll(ms().romPath[1], "fat:/", slashchar);
					fcrompathini.SetString("YSMENU", "AUTO_BOOT", path);
					fcrompathini.SaveIniFile("fat:/TTMenu/YSMenu.ini");
					return runNdsFile("fat:/YSMenu.nds", 0, NULL, true, true, true, runNds_boostCpu, runNds_boostVram, false, -1);
				}
			}
		}
		case TWLSettings::ESDFlashcardDirectLaunch: {
			ms().romfolder[ms().previousUsedDevice] = ms().romPath[ms().previousUsedDevice];
			while (!ms().romfolder[ms().previousUsedDevice].empty() && ms().romfolder[ms().previousUsedDevice][ms().romfolder[ms().previousUsedDevice].size()-1] != '/') {
				ms().romfolder[ms().previousUsedDevice].resize(ms().romfolder[ms().previousUsedDevice].size()-1);
			}
			chdir(ms().romfolder[ms().previousUsedDevice].c_str());

			std::string filename = ms().romPath[ms().previousUsedDevice];
			const size_t last_slash_idx = filename.find_last_of("/");
			if (std::string::npos != last_slash_idx) {
				filename.erase(0, last_slash_idx + 1);
			}

			argarray.push_back((char*)ms().romPath[ms().previousUsedDevice].c_str());

			char game_TID[5];

			FILE *f_nds_file = fopen(filename.c_str(), "rb");
			fseek(f_nds_file, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
			fread(game_TID, 1, 4, f_nds_file);
			game_TID[4] = 0;

			fclose(f_nds_file);

			loadPerGameSettings(filename);

			int runNds_language = perGameSettings_language == -2 ? ms().getGameLanguage() : perGameSettings_language;
			int runNds_gameRegion = perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region;

			// Set region flag
			if (ms().useRomRegion && perGameSettings_region < -1 && game_TID[3] != 'A' && game_TID[3] != 'O' && game_TID[3] != '#') {
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
			} else if (runNds_gameRegion == -1) {
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
				*(u8*)(0x02FFFD70) = runNds_gameRegion;
			}

			if (runNds_language >= 0 && runNds_language <= 7 && *(u8*)0x02000406 != runNds_language) {
				tonccpy((char*)0x02000600, (char*)0x02000400, 0x200);
				*(u8*)0x02000606 = runNds_language;
				*(u32*)0x02FFFDFC = 0x02000600;
			}

			bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);

			if (ms().consoleModel >= 2 && twlBgCxiFound && useWidescreen && ms().homebrewHasWide) {
				argarray.push_back((char*)"wide");
			}

			bool runNds_boostCpu = perGameSettings_boostCpu == -1 ? DEFAULT_BOOST_CPU : perGameSettings_boostCpu;
			bool runNds_boostVram = perGameSettings_boostVram == -1 ? DEFAULT_BOOST_VRAM : perGameSettings_boostVram;

			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, (!perGameSettings_dsiMode ? true : false), runNds_boostCpu, runNds_boostVram, false, runNds_language);
		} case TWLSettings::EDSiWareLaunch: {
			ms().romfolder[ms().previousUsedDevice] = ms().romPath[ms().previousUsedDevice];
			while (!ms().romfolder[ms().previousUsedDevice].empty() && ms().romfolder[ms().previousUsedDevice][ms().romfolder[ms().previousUsedDevice].size()-1] != '/') {
				ms().romfolder[ms().previousUsedDevice].resize(ms().romfolder[ms().previousUsedDevice].size()-1);
			}
			chdir(ms().romfolder[ms().previousUsedDevice].c_str());

			std::string filename = ms().romPath[ms().previousUsedDevice];
			const size_t last_slash_idx = filename.find_last_of("/");
			if (std::string::npos != last_slash_idx) {
				filename.erase(0, last_slash_idx + 1);
			}

			loadPerGameSettings(filename);
			if ((perGameSettings_dsiwareBooter == -1 ? ms().dsiWareBooter : perGameSettings_dsiwareBooter) || ms().consoleModel >= 2) {
				if (ms().homebrewBootstrap) {
					unlaunchBootDSiWare();
				} else {
					bool useNightly = (perGameSettings_bootstrapFile == -1 ? ms().bootstrapFile : perGameSettings_bootstrapFile);
					if (*(u32*)(0x02000000) & BIT(3)) {
						useNightly = *(bool*)(0x02000010);
					}

					bool useWidescreen = (perGameSettings_wideScreen == -1 ? ms().wideScreen : perGameSettings_wideScreen);
					if (*(u32*)(0x02000000) & BIT(3)) {
						useWidescreen = *(bool*)(0x02000014);
					}

					wideCheck(useWidescreen);

					char ndsToBoot[256];
					sprintf(ndsToBoot, "sd:/_nds/nds-bootstrap-%s.nds", useNightly ? "nightly" : "release");
					if (access(ndsToBoot, F_OK) != 0) {
						sprintf(ndsToBoot, "fat:/_nds/nds-bootstrap-%s.nds", useNightly ? "nightly" : "release");
					}

					argarray.push_back(ndsToBoot);

				  if (!(*(u32*)(0x02000000) & BIT(3))) {
					char sfnSrl[62];
					char sfnPub[62];
					char sfnPrv[62];
					if (ms().previousUsedDevice && ms().dsiWareToSD) {
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

					const char *bootstrapinipath = BOOTSTRAP_INI;
					CIniFile bootstrapini(bootstrapinipath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", ms().previousUsedDevice && ms().dsiWareToSD ? "sd:/_nds/TWiLightMenu/tempDSiWare.dsi" : ms().dsiWareSrlPath);
					bootstrapini.SetString("NDS-BOOTSTRAP", "APP_PATH", sfnSrl);
					bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", sfnPub);
					bootstrapini.SetString("NDS-BOOTSTRAP", "PRV_PATH", sfnPrv);
					bootstrapini.SetString("NDS-BOOTSTRAP", "AP_FIX_PATH", "");
					// bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
					bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", (perGameSettings_language == -2 ? ms().getGameLanguage() : perGameSettings_language));
					bootstrapini.SetInt("NDS-BOOTSTRAP", "REGION", (perGameSettings_region < -1 ? ms().gameRegion : perGameSettings_region));
					bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", true);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "DONOR_SDK_VER", 5);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "GAME_SOFT_RESET", 1);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_REGION", 0);
					bootstrapini.SetInt("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", 0);
					bootstrapini.SaveIniFile(bootstrapinipath);
				  }

					return runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);
				}
			} else {
				ms().romfolder[ms().previousUsedDevice] = ms().romPath[ms().previousUsedDevice];
					while (!ms().romfolder[ms().previousUsedDevice].empty() && ms().romfolder[ms().previousUsedDevice][ms().romfolder[ms().previousUsedDevice].size()-1] != '/') {
						ms().romfolder[ms().previousUsedDevice].resize(ms().romfolder[ms().previousUsedDevice].size()-1);
					}
					chdir(ms().romfolder[ms().previousUsedDevice].c_str());

					std::string filename = ms().romPath[ms().previousUsedDevice];

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

				// Move .pub and/or .prv out of "saves" folder
				std::string pubnameUl = ReplaceAll(filename, typeToReplace, ".pub");
				std::string prvnameUl = ReplaceAll(filename, typeToReplace, ".prv");
				std::string pubpathUl = ms().romfolder[ms().previousUsedDevice] + "/" + pubnameUl;
				std::string prvpathUl = ms().romfolder[ms().previousUsedDevice] + "/" + prvnameUl;
				if (access(ms().dsiWarePubPath.c_str(), F_OK) == 0) {
					rename(ms().dsiWarePubPath.c_str(), pubpathUl.c_str());
				}
				if (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) {
					rename(ms().dsiWarePrvPath.c_str(), prvpathUl.c_str());
				}

				unlaunchBootDSiWare();
			}
			break;
		} case TWLSettings::ENESDSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/nestwl.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to nesDS as argument
		case TWLSettings::EGameYobLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/gameyob.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to GameYob as argument
		case TWLSettings::ES8DSLaunch:
			mkdir("sd:/data", 0777);
			mkdir("sd:/data/s8ds", 0777);
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/S8DS.nds";
			return runNdsFile(argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to S8DS as argument
		case TWLSettings::ERVideoLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/RocketVideoPlayer.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass video to Rocket Video Player as argument
		case TWLSettings::EFastVideoLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/FastVideoDS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass video to FastVideoDS as argument
		case TWLSettings::EStellaDSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/StellaDS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to StellaDS as argument
		case TWLSettings::EPicoDriveTWLLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/PicoDriveTWL.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to PicoDrive TWL as argument
		case TWLSettings::EA7800DSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/A7800DS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to A7800DS as argument
		case TWLSettings::EA5200DSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/A5200DS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to A5200DS as argument
		case TWLSettings::ENitroGrafxLaunch:
			mkdir("sd:/data", 0777);
			mkdir("sd:/data/NitroGrafx", 0777);
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NitroGrafx.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to NitroGrafx as argument
		case TWLSettings::EXEGSDSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/XEGS-DS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to XEGS-DS as argument
		case TWLSettings::ENINTVDSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NINTV-DS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to NINTV-DS as argument
		case TWLSettings::EGBARunner2Launch:
			argarray.at(0) = (char*)(ms().consoleModel > 0 ? "sd:/_nds/GBARunner2_arm7dldi_3ds.nds" : "sd:/_nds/GBARunner2_arm7dldi_dsi.nds");
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1);	// Pass ROM to GBARunner2 as argument
		case TWLSettings::EColecoDSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/ColecoDS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to ColecoDS as argument
		case TWLSettings::ENitroSwanLaunch:
			mkdir("sd:/data", 0777);
			mkdir("sd:/data/nitroswan", 0777);
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NitroSwan.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to NitroSwan as argument
		case TWLSettings::ENGPDSLaunch:
			mkdir("sd:/data", 0777);
			mkdir("sd:/data/ngpds", 0777);
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/NGPDS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to NGPDS as argument
		case TWLSettings::ESNEmulDSLaunch:
			{
				const char* ndsToBoot = (char*)"sd:/_nds/TWiLightMenu/emulators/SNEmulDS.srl";
				if (!isDSiMode() || access(argarray[0], F_OK) != 0) {
					ndsToBoot = (char*)"fat:/_nds/TWiLightMenu/emulators/SNEmulDS.srl";
				}
				argarray.at(0) = (char*)"fat:/SNEmulDS.srl";
				return runNdsFile(ndsToBoot, argarray.size(), (const char **)&argarray[0], true, true, false, true, true, true, -1); // Pass ROM to SNEmulDS as argument
			}
		case TWLSettings::EGBANativeLaunch:
		case TWLSettings::ENoLaunch:
			break;
		case TWLSettings::EAmEDSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/AmEDS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to AmEDS as argument
		case TWLSettings::ECrocoDSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/emulators/CrocoDS.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass ROM to CrocoDS as argument
		case TWLSettings::ETunaViDSLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/apps/tuna-vids.nds";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass video to tuna-viDS as argument
		case TWLSettings::EImageLaunch:
			argarray.at(0) = (char*)"sd:/_nds/TWiLightMenu/imageview.srldr";
			return runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0], true, true, false, true, true, false, -1); // Pass image to image viewer as argument
	}
	
	return -1;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------
	fifoSendValue32(FIFO_PM, PM_REQ_SLEEP_DISABLE);

	// overwrite reboot stub identifier
	extern char *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

	useTwlCfg = (dsiFeatures() && (*(u8*)0x02000400 != 0) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));

	//consoleDemoInit();

	extern const DISC_INTERFACE __my_io_dsisd;

	//iprintf("Initing FAT...\n");
	if (!fatMountSimple("sd", &__my_io_dsisd)) {
		consoleDemoInit();
		iprintf("FAT init failed!");
		stop();
	}
	chdir("sd:/");

	//iprintf("Initing NitroFS...\n");
	if (!nitroFSInit(argc==0 ? "sd:/_nds/TWiLightMenu/resetgame.srldr" : argv[0])) {
		consoleDemoInit();
		iprintf("NitroFS init failed!");
		stop();
	}

	if (*(u32*)(0x02000004) != 0) {
		*(u32*)(0x02000000) = 0; // Clear soft-reset params
	}

	flashcardInit();

	//iprintf("Waiting on FIFO_USER_06...\n");
	fifoWaitValue32(FIFO_USER_06);

	//iprintf("lastRunROM()\n");
	int err = lastRunROM();
	consoleDemoInit();
	iprintf ("Start failed. Error %i", err);
	stop();

	return 0;
}
