/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#include <nds.h>
#include <maxmod9.h>

#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>
#include <gl2d.h>

#include "graphics/graphics.h"

#include "ndsLoaderArm9.h"
#include "fileBrowse.h"

#include "dsmenu_top.h"

#include "iconTitle.h"
#include "graphics/fontHandler.h"

#include "inifile.h"

bool whiteScreen = false;

const char* settingsinipath = "sd:/_nds/srloader/settings.ini";
const char* bootstrapinipath = "sd:/_nds/nds-bootstrap.ini";

/**
 * Remove trailing slashes from a pathname, if present.
 * @param path Pathname to modify.
 */
static void RemoveTrailingSlashes(std::string& path)
{
	while (!path.empty() && path[path.size()-1] == '/') {
		path.resize(path.size()-1);
	}
}

std::string romfolder;
std::string gbromfolder;

std::string arm7DonorPath;

int donorSdkVer = 0;

bool run_timeout = true;

int mpuregion = 0;
int mpusize = 0;

int romtype = 0;

bool applaunch = false;

bool gotosettings = false;

bool ntr_touch = true;

bool autorun = false;
int theme = 0;
int subtheme = 0;

int flashcardUsed = false;

int flashcard;
/* Flashcard value
	0: DSTT/R4i Gold/R4i-SDHC/R4 SDHC Dual-Core/R4 SDHC Upgrade/SC DSONE
	1: R4DS (Original Non-SDHC version)/ M3 Simply
	2: R4iDSN/R4i Gold RTS
	3: Acekard 2(i)/Galaxy Eagle/M3DS Real
	4: Acekard RPG
	5: Ace 3DS+/Gateway Blue Card/R4iTT
	6: SuperCard DSTWO
*/

void LoadSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	// UI settings.
	if (flashcardUsed) romfolder = settingsini.GetString("SRLOADER", "FCROM_FOLDER", "");
	else romfolder = settingsini.GetString("SRLOADER", "ROM_FOLDER", "");
	RemoveTrailingSlashes(romfolder);
	gbromfolder = settingsini.GetString("SRLOADER", "GBROM_FOLDER", "");
	RemoveTrailingSlashes(gbromfolder);
	if (flashcardUsed) {
		romtype = 0;
	} else {
		romtype = settingsini.GetInt("SRLOADER", "ROM_TYPE", 0);
	}

	// Customizable UI settings.
	autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", 0);
	gotosettings = settingsini.GetInt("SRLOADER", "GOTOSETTINGS", 0);
	theme = settingsini.GetInt("SRLOADER", "THEME", 1);
	subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", 0);

	ntr_touch = settingsini.GetInt("SRLOADER", "NTR_TOUCH", 1);
	flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", 0);

	// nds-bootstrap
	CIniFile bootstrapini( bootstrapinipath );
	
	arm7DonorPath = bootstrapini.GetString( "NDS-BOOTSTRAP", "ARM7_DONOR_PATH", "");
}

void SaveSettings(void) {
	// GUI
	CIniFile settingsini( settingsinipath );

	if (!flashcardUsed) {
		settingsini.SetInt("SRLOADER", "ROM_TYPE", romtype);
	}

	// UI settings.
	settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
	settingsini.SetInt("SRLOADER", "GOTOSETTINGS", gotosettings);
	settingsini.SetInt("SRLOADER", "NTR_TOUCH", ntr_touch);
	settingsini.SetInt("SRLOADER", "FLASHCARD", flashcard);
	settingsini.SetInt("SRLOADER", "THEME", theme);
	settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
	settingsini.SaveIniFile(settingsinipath);

	// nds-bootstrap
	CIniFile bootstrapini( bootstrapinipath );
	
	bootstrapini.SetString( "NDS-BOOTSTRAP", "ARM7_DONOR_PATH", arm7DonorPath);
	bootstrapini.SaveIniFile(bootstrapinipath);
}

bool useBootstrap = false;

using namespace std;

typedef struct {
	char gameTitle[12];			//!< 12 characters for the game title.
	char gameCode[4];			//!< 4 characters for the game code.
} sNDSHeadertitlecodeonly;

/**
 * Set donor SDK version for a specific game.
 */
void SetDonorSDK(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	game_TID[4] = 0;
	game_TID[3] = 0;
	fclose(f_nds_file);
	
	donorSdkVer = 0;

	// Check for ROM hacks that need an SDK version.
	static const char sdk2_list[][4] = {
		"AMQ",	// Mario vs. Donkey Kong 2 - March of the Minis
		"AMH",	// Metroid Prime Hunters
		"ASM",	// Super Mario 64 DS
	};
	
	static const char sdk3_list[][4] = {
		"AMC",	// Mario Kart DS
		"EKD",	// Ermii Kart DS (Mario Kart DS hack)
		"A2D",	// New Super Mario Bros.
		"ADA",	// Pokemon Diamond
		"APA",	// Pokemon Pearl
		"ARZ",	// Rockman ZX/MegaMan ZX
		"YZX",	// Rockman ZX Advent/MegaMan ZX Advent
		"B6Z",	// Rockman Zero Collection/MegaMan Zero Collection
	};
	
	static const char sdk4_list[][4] = {
		"A6C",	// MegaMan Star Force - Dragon
		"B6Z",	// Rockman Zero Collection/MegaMan Zero Collection
		"YT7",	// SEGA Superstars Tennis
		"AZL",	// Style Savvy
	};

	static const char sdk5_list[][4] = {
		"CS3",	// Sonic and Sega All Stars Racing
		"BXS",	// Sonic Colo(u)rs
	};

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(sdk2_list)/sizeof(sdk2_list[0]); i++) {
		if (!memcmp(game_TID, sdk2_list[i], 3)) {
			// Found a match.
			donorSdkVer = 2;
			break;
		}
	}
	
	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(sdk3_list)/sizeof(sdk3_list[0]); i++) {
		if (!memcmp(game_TID, sdk3_list[i], 3)) {
			// Found a match.
			donorSdkVer = 3;
			break;
		}
	}

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(sdk4_list)/sizeof(sdk4_list[0]); i++) {
		if (!memcmp(game_TID, sdk4_list[i], 3)) {
			// Found a match.
			donorSdkVer = 4;
			break;
		}
	}

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(sdk5_list)/sizeof(sdk5_list[0]); i++) {
		if (!memcmp(game_TID, sdk5_list[i], 3)) {
			// Found a match.
			donorSdkVer = 5;
			break;
		}
	}

}

/**
 * Set compatibility check for a specific game.
 */
void SetCompatibilityCheck(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	game_TID[4] = 0;
	game_TID[3] = 0;
	fclose(f_nds_file);
	
	run_timeout = true;

	// Check for games that don't need compatibility checks.
	static const char list[][4] = {
		"###",	// Homebrew
		"NTR",	// Download Play ROMs
		"ADM",	// Animal Crossing: Wild World
		"AZD",	// The Legend of Zelda: Twilight Princess E3 Trailer
		"A2D",	// New Super Mario Bros.
	};
	
	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(list)/sizeof(list[0]); i++) {
		if (!memcmp(game_TID, list[i], 3)) {
			// Found a match.
			run_timeout = false;
			break;
		}
	}
}

/**
 * Set MPU settings for a specific game.
 */
void SetMPUSettings(const char* filename) {
	FILE *f_nds_file = fopen(filename, "rb");

	char game_TID[5];
	fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
	fread(game_TID, 1, 4, f_nds_file);
	game_TID[4] = 0;
	game_TID[3] = 0;
	fclose(f_nds_file);

	scanKeys();
	int pressed = keysDownRepeat();
	
	if(pressed & KEY_B){
		mpuregion = 1;
	} else if(pressed & KEY_X){
		mpuregion = 2;
	} else if(pressed & KEY_Y){
		mpuregion = 3;
	} else {
		mpuregion = 0;
	}
	if(pressed & KEY_RIGHT){
		mpusize = 3145728;
	} else if(pressed & KEY_LEFT){
		mpusize = 1;
	} else {
		mpusize = 0;
	}

	// Check for games that need an MPU size of 3 MB.
	static const char mpu_3MB_list[][4] = {
		"A7A",	// DS Download Station - Vol 1
		"A7B",	// DS Download Station - Vol 2
		"A7C",	// DS Download Station - Vol 3
		"A7D",	// DS Download Station - Vol 4
		"A7E",	// DS Download Station - Vol 5
		"A7F",	// DS Download Station - Vol 6 (EUR)
		"A7G",	// DS Download Station - Vol 6 (USA)
		"A7H",	// DS Download Station - Vol 7
		"A7I",	// DS Download Station - Vol 8
		"A7J",	// DS Download Station - Vol 9
		"A7K",	// DS Download Station - Vol 10
		"A7L",	// DS Download Station - Vol 11
		"A7M",	// DS Download Station - Vol 12
		"A7N",	// DS Download Station - Vol 13
		"A7O",	// DS Download Station - Vol 14
		"A7P",	// DS Download Station - Vol 15
		"A7Q",	// DS Download Station - Vol 16
		"A7R",	// DS Download Station - Vol 17
		"A7S",	// DS Download Station - Vol 18
		"A7T",	// DS Download Station - Vol 19
		"ARZ",	// Rockman ZX/MegaMan ZX
		"YZX",	// Rockman ZX Advent/MegaMan ZX Advent
		"B6Z",	// Rockman Zero Collection/MegaMan Zero Collection
		"A2D",	// New Super Mario Bros.
	};

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(mpu_3MB_list)/sizeof(mpu_3MB_list[0]); i++) {
		if (!memcmp(game_TID, mpu_3MB_list[i], 3)) {
			// Found a match.
			mpuregion = 1;
			mpusize = 3145728;
			break;
		}
	}
}

//---------------------------------------------------------------------------------
void stop (void) {
//---------------------------------------------------------------------------------
	while (1) {
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause(int x, int y) {
//---------------------------------------------------------------------------------
	// iprintf("Press start...\n");
	printSmall(false, x, y, "Press start...");
	while(1) {
		scanKeys();
		if(keysDown() & KEY_START)
			break;
		swiWaitForVBlank();
	}
	scanKeys();
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

	// overwrite reboot stub identifier
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	defaultExceptionHandler();

	// Read user name
	char *username = (char*)PersonalData->name;
		
	// text
	for (int i = 0; i < 10; i++) {
		if (username[i*2] == 0x00)
			username[i*2/2] = 0;
		else
			username[i*2/2] = username[i*2];
	}

#ifndef EMULATE_FILES

	if (!fatInitDefault()) {
		graphicsInit();
		fontInit();
		printSmall(true, 1, 2, username);
		printSmall(false, 4, 4, "fatinitDefault failed!");
		stop();
	}

#endif

	std::string filename;
	std::string bootstrapfilename;

	if (!access("fat:/", F_OK)) flashcardUsed = true;

	LoadSettings();
	
	graphicsInit();
	fontInit();

	iconTitleInit();
	
	keysSetRepeat(25,5);

	vector<string> extensionList;
	vector<string> gbExtensionList;
	extensionList.push_back(".nds");
	extensionList.push_back(".argv");
	gbExtensionList.push_back(".gb");
	gbExtensionList.push_back(".sgb");
	gbExtensionList.push_back(".gbc");
	srand(time(NULL));
	
	if (romfolder == "") romfolder = "roms/nds";
	if (gbromfolder == "") gbromfolder = "roms/gb";
	
	char path[256];
	snprintf (path, sizeof(path), "sd:/%s", romfolder.c_str());
	char gbPath[256];
	snprintf (gbPath, sizeof(gbPath), "sd:/%s", gbromfolder.c_str());

	InitSound();
	
	while(1) {
	
		if (romtype == 1) {
			// Set directory
			chdir (gbPath);

			//Navigates to the file to launch
			filename = browseForFile(gbExtensionList, username);
		} else {
			// Set directory
			chdir (path);

			//Navigates to the file to launch
			filename = browseForFile(extensionList, username);
		}

		////////////////////////////////////
		// Launch the item
#ifndef EMULATE_FILES

		if (applaunch) {		
			// Construct a command line
			getcwd (filePath, PATH_MAX);
			int pathLen = strlen(filePath);
			vector<char*> argarray;

			if ( strcasecmp (filename.c_str() + filename.size() - 5, ".argv") == 0) {

				FILE *argfile = fopen(filename.c_str(),"rb");
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
				filename = argarray.at(0);
			} else {
				argarray.push_back(strdup(filename.c_str()));
			}

			if ( strcasecmp (filename.c_str() + filename.size() - 4, ".nds") == 0 ) {
				char *name = argarray.at(0);
				strcpy (filePath + pathLen, name);
				free(argarray.at(0));
				argarray.at(0) = filePath;
				if (useBootstrap) {
					char game_TID[5];
					
					FILE *f_nds_file = fopen(argarray[0], "rb");

					fseek(f_nds_file, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
					fread(game_TID, 1, 4, f_nds_file);
					game_TID[4] = 0;
					game_TID[3] = 0;
					fclose(f_nds_file);

					std::string savename = ReplaceAll(argarray[0], ".nds", ".sav");

					if (access(savename.c_str(), F_OK)) {
						if (strcmp(game_TID, "###") != 0) {	// Create save if game isn't homebrew
							printSmall(false, 8, 20, "Creating save file...");

							static const int BUFFER_SIZE = 4096;
							char buffer[BUFFER_SIZE];
							memset(buffer, 0, sizeof(buffer));

							int savesize = 524288;	// 512KB (default size for most games)

							// Set save size to 8KB for the following games
							if (strcmp(game_TID, "ASC") == 0 )	// Sonic Rush
							{
								savesize = 8192;
							}

							// Set save size to 256KB for the following games
							if (strcmp(game_TID, "AMH") == 0 )	// Metroid Prime Hunters
							{
								savesize = 262144;
							}

							// Set save size to 1MB for the following games
							if (strcmp(game_TID, "AZL") == 0 )	// Wagamama Fashion: Girls Mode/Style Savvy/Nintendo presents: Style Boutique/Namanui Collection: Girls Style
							{
								savesize = 1048576;
							}

							// Set save size to 32MB for the following games
							if (strcmp(game_TID, "UOR") == 0 )	// WarioWare - D.I.Y. (Do It Yourself)
							{
								savesize = 1048576*32;
							}

							FILE *pFile = fopen(savename.c_str(), "wb");
							if (pFile) {
								for (int i = savesize; i > 0; i -= BUFFER_SIZE) {
									fwrite(buffer, 1, sizeof(buffer), pFile);
								}
								fclose(pFile);
							}
							printSmall(false, 8, 28, "Save file created!");
						}

					}
					
					SetDonorSDK(argarray[0]);
					SetCompatibilityCheck(argarray[0]);
					SetMPUSettings(argarray[0]);
					
					std::string path = argarray[0];
					std::string savepath = savename;
					CIniFile bootstrapini( "sd:/_nds/nds-bootstrap.ini" );
					bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", path);
					bootstrapini.SetString("NDS-BOOTSTRAP", "SAV_PATH", savepath);
					bootstrapini.SetInt( "NDS-BOOTSTRAP", "DONOR_SDK_VER", donorSdkVer);
					bootstrapini.SetInt( "NDS-BOOTSTRAP", "CHECK_COMPATIBILITY", run_timeout);
					bootstrapini.SetInt( "NDS-BOOTSTRAP", "PATCH_MPU_REGION", mpuregion);
					bootstrapini.SetInt( "NDS-BOOTSTRAP", "PATCH_MPU_SIZE", mpusize);
					bootstrapini.SaveIniFile( "sd:/_nds/nds-bootstrap.ini" );
					if (strcmp(game_TID, "###") == 0) {
						bootstrapfilename = "sd:/_nds/hb-bootstrap.nds";
					} else {
						if (fifoGetValue32(FIFO_USER_03) != 0) {
							if(ntr_touch) bootstrapfilename = "sd:/_nds/rocket-bootstrap.nds";
							else bootstrapfilename = "sd:/_nds/nds-bootstrap.nds";
						} else {
							bootstrapfilename = "sd:/_nds/dsiware-bootstrap.nds";
						}
					}
					int err = runNdsFile (bootstrapfilename.c_str(), 0, 0);
					char text[32];
					snprintf (text, sizeof(text), "Start failed. Error %i", err);
					printSmall(false, 8, 20, text);
					stop();
				} else {
					iprintf ("Running %s with %d parameters\n", argarray[0], argarray.size());
					int err = runNdsFile (argarray[0], argarray.size(), (const char **)&argarray[0]);
					char text[32];
					snprintf (text, sizeof(text), "Start failed. Error %i", err);
					printSmall(false, 8, 20, text);
					stop();
				}
			} else if ( strcasecmp (filename.c_str() + filename.size() - 3, ".gb") == 0 ||
						strcasecmp (filename.c_str() + filename.size() - 4, ".sgb") == 0 ||
						strcasecmp (filename.c_str() + filename.size() - 4, ".gbc") == 0 ) {
				char gbROMpath[256];
				snprintf (gbROMpath, sizeof(gbROMpath), "/%s/%s", gbromfolder.c_str(), filename.c_str());
				argarray.push_back(gbROMpath);
				argarray.at(0) = "sd:/_nds/srloader/emulators/gameyob.nds";
				int err = runNdsFile ("sd:/_nds/srloader/emulators/gameyob.nds", argarray.size(), (const char **)&argarray[0]);	// Pass ROM to GameYob as argument
				char text[32];
				snprintf (text, sizeof(text), "Start failed. Error %i", err);
				printSmall(false, 8, 20, text);
				stop();
			} else {

			}

			while(argarray.size() !=0 ) {
				free(argarray.at(0));
				argarray.erase(argarray.begin());
			}

			// while (1) {
			// 	swiWaitForVBlank();
			// 	scanKeys();
			// 	if (!(keysHeld() & KEY_A)) break;
			// }
		}
#endif
	}

	return 0;
}
