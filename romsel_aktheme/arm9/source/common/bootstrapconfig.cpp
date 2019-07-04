#include "systemdetails.h"
#include "bootstrapconfig.h"
#include "dsimenusettings.h"
#include "filecopy.h"
#include "flashcard.h"
#include "loaderconfig.h"
#include "systemfilenames.h"
#include "tool/stringtool.h"
#include "windows/cheatwnd.h"
#include "windows/mainlist.h"
#include <stdio.h>
#include <nds.h>
#include <nds/arm9/dldi.h>

extern std::string getSavExtension(int number);
extern std::string getImgExtension(int number);

BootstrapConfig::BootstrapConfig(const std::string &fileName, const std::string &fullPath, const std::string &gametid, u32 sdkVersion)
	: _fileName(fileName), _fullPath(fullPath), _gametid(gametid), _sdkVersion(sdkVersion)
{
	_donorSdk = 0;
	_mpuSize = 0;
	_mpuRegion = 0;
	_ceCached = false;
	_forceSleepPatch = (ms().forceSleepPatch || (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS())) ? true : false;
	_isHomebrew = _gametid.empty() || _sdkVersion == 0;
	_saveSize = 0x80000;
	_dsiMode = 0;
	_vramBoost = false;
	_language = -1;
	_saveNo = 0;
	_ramDiskNo = -1;
	_softReset = false;
	_soundFix = false;
	_cpuBoost = false;
	_useGbarBootstrap = false;

	this
		->saveSize()
		.softReset()
		.mpuSettings()
		.speedBumpExclude()
		.donorSdk();
}

BootstrapConfig::~BootstrapConfig()
{
}

BootstrapConfig &BootstrapConfig::saveSize()
{
	if (strncmp("ASC", _gametid.c_str(), 3) == 0)
	{
		return saveSize(0x2000);
	}
	if (strncmp("AMH", _gametid.c_str(), 3) == 0)
	{
		return saveSize(0x40000);
	}

	if (strncmp("AZL", _gametid.c_str(), 3) == 0 || strncmp("BKI", _gametid.c_str(), 3) == 0)
	{
		return saveSize(0x100000);
	}

	if (strncmp("UOR", _gametid.c_str(), 3) == 0 || strncmp("UXB", _gametid.c_str(), 3) == 0)
	{
		return saveSize(0x2000000);
	}

	return saveSize(0x80000);
}

BootstrapConfig &BootstrapConfig::softReset()
{
	static const char list[][4] = {
		"NTR", // Download Play ROMs
		"ASM", // Super Mario 64 DS
		"SMS", // Super Mario Star World, and Mario's Holiday
		"AMC", // Mario Kart DS
		"EKD", // Ermii Kart DS
		"A2D", // New Super Mario Bros.
		"ARZ", // Rockman ZX/MegaMan ZX
		"AKW", // Kirby Squeak Squad/Mouse Attack
		"YZX", // Rockman ZX Advent/MegaMan ZX Advent
		"B6Z", // Rockman Zero Collection/MegaMan Zero Collection
	};
	for (const char *resettid : list)
	{
		if (strncmp(resettid, _gametid.c_str(), 3) == 0)
		{
			return softReset(true);
		}
	}
	return softReset(false);
}
BootstrapConfig &BootstrapConfig::donorSdk(int sdk)
{
	_donorSdk = sdk;
	return *this;
}

BootstrapConfig &BootstrapConfig::mpuSettings()
{
	static const char mpu_3MB_list[][4] = {
		"A7A", // DS Download Station - Vol 1
		"A7B", // DS Download Station - Vol 2
		"A7C", // DS Download Station - Vol 3
		"A7D", // DS Download Station - Vol 4
		"A7E", // DS Download Station - Vol 5
		"A7F", // DS Download Station - Vol 6 (EUR)
		"A7G", // DS Download Station - Vol 6 (USA)
		"A7H", // DS Download Station - Vol 7
		"A7I", // DS Download Station - Vol 8
		"A7J", // DS Download Station - Vol 9
		"A7K", // DS Download Station - Vol 10
		"A7L", // DS Download Station - Vol 11
		"A7M", // DS Download Station - Vol 12
		"A7N", // DS Download Station - Vol 13
		"A7O", // DS Download Station - Vol 14
		"A7P", // DS Download Station - Vol 15
		"A7Q", // DS Download Station - Vol 16
		"A7R", // DS Download Station - Vol 17
		"A7S", // DS Download Station - Vol 18
		"A7T", // DS Download Station - Vol 19
		"ARZ", // Rockman ZX/MegaMan ZX
		"YZX", // Rockman ZX Advent/MegaMan ZX Advent
		"B6Z", // Rockman Zero Collection/MegaMan Zero Collection
		"A2D", // New Super Mario Bros.
	};
	for (const char *mputid : mpu_3MB_list)
	{
		if (strncmp(mputid, _gametid.c_str(), 3) == 0)
		{
			return mpuRegion(1).mpuSize(0x300000);
		}
	}
	return mpuRegion(0).mpuSize(0);
}
BootstrapConfig &BootstrapConfig::speedBumpExclude()
{
	scanKeys();
	if(keysHeld() & KEY_L){
		return ceCached(false);
	}

	static const char list[][5] = {
		"AWRP",	// Advance Wars: Dual Strike (EUR)
		"AVCP",	// Magical Starsign (EUR)
		"YFTP",	// Pokemon Mystery Dungeon: Explorers of Time (EUR)
		"YFYP",	// Pokemon Mystery Dungeon: Explorers of Darkness (EUR)
		"AH9P",	// Tony Hawk's American Sk8land (EUR)
	};
	for (const char *speedtid : list)
	{
		if (strncmp(speedtid, _gametid.c_str(), 4) == 0)
		{
			return ceCached(false);
		}
	}

	static const char list2[][4] = {
		"AEK",	// Age of Empires: The Age of Kings
		"ALC",	// Animaniacs: Lights, Camera, Action!
		"YAH",	// Assassin's Creed: Altaïr's Chronicles
		"AB2",	// Battles of Prince of Persia
		"YB4",	// Bee Movie
		"CBK",	// Bolt
		"CBD",	// Bolt: Be-Awesome Edition
		//"ACV",	// Castlevania: Dawn of Sorrow	(fixed on nds-bootstrap side)
		"YIV",	// Dragon Quest IV: Chapters of the Chosen
		"AE4",	// Eyeshield 21 Max Devil Power
		"APR",	// Feel the Magic - XY XX
		"A26",	// Feel the Magic - XY XX (Demo)
		"A5P",	// Harry Potter and the Order of the Phoenix
		"CQ7",	// Henry Hatsworth
		"AR2",	// Kirarin * Revolution: Naasan to Issho
		"ARM",	// Mario & Luigi: Partners in Time
		"CLJ",	// Mario & Luigi: Bowser's Inside Story
		"COL",	// Mario & Sonic at the Olympic Winter Games
		"AMQ",	// Mario vs. Donkey Kong 2: March of the Minis
		"AMH",	// Metroid Prime Hunters
		"B2K",	// Ni no Kuni: Shikkoku no Madoushi
		"C2S",	// Pokemon Mystery Dungeon: Explorers of Sky
		"Y6S",	// Pokemon Mystery Dungeon: Explorers of Sky (Demo)
		"B3R",	// Pokemon Ranger: Guardian Signs
		"APU",	// Puyo Puyo!! 15th Anniversary
		"BYO",	// Puyo Puyo 7
		"YZX",	// Rockman ZX Advent/MegaMan ZX Advent
	    "B6X",	// Rockman EXE: Operate Shooting Star
	    "B6Z",	// Rockman Zero Collection/MegaMan Zero Collection
		"AKA",	// The Rub Rabbits!
		"ARF",	// Rune Factory: A Fantasy Harvest Moon
		"A6N",	// Rune Factory 2: A Fantasy Harvest Moon
		"A3S",	// Shrek the Third
		"AIR",	// Space Invaders DS
		"AIS",	// Space Invaders Revolution
		"YV4",  // Spectrobes: Beyond the Portals
		"AS2",  // Spider-Man 2
		"AQ3",	// Spider-Man 3
		"AST",	// Star Wars Episode III: Revenge of the Sith
		"CS7",	// Summon Night X: Tears Crown
		"AYT",	// Tales of Innocence
		"YT9",	// Tony Hawk's Proving Ground
		"YYK",	// Trauma Center: Under the Knife 2
		"CY8",	// Yu-Gi-Oh! World Championship 2009
		"BYX",	// Yu-Gi-Oh! World Championship 2010
	};
	for (const char *speedtid : list2)
	{
		if (strncmp(speedtid, _gametid.c_str(), 3) == 0)
		{
			return ceCached(false);
		}
	}
	return ceCached(true);
}
BootstrapConfig &BootstrapConfig::donorSdk()
{
	static const char sdk2_list[][4] = {
		"AMQ", // Mario vs. Donkey Kong 2 - March of the Minis
		"AMH", // Metroid Prime Hunters
		"ASM", // Super Mario 64 DS
	};

	static const char sdk3_list[][4] = {
		"AMC", // Mario Kart DS
		"EKD", // Ermii Kart DS (Mario Kart DS hack)
		"A2D", // New Super Mario Bros.
		"ADA", // Pokemon Diamond
		"APA", // Pokemon Pearl
		"ARZ", // Rockman ZX/MegaMan ZX
		"YZX", // Rockman ZX Advent/MegaMan ZX Advent
	};

	static const char sdk4_list[][4] = {
		"YKW", // Kirby Super Star Ultra
		"A6C", // MegaMan Star Force: Dragon
		"A6B", // MegaMan Star Force: Leo
		"A6A", // MegaMan Star Force: Pegasus
		"B6Z", // Rockman Zero Collection/MegaMan Zero Collection
		"YT7", // SEGA Superstars Tennis
		"AZL", // Style Savvy
		"BKI", // The Legend of Zelda: Spirit Tracks
		"B3R", // Pokemon Ranger: Guardian Signs
	};

	static const char sdk5_list[][4] = {
		"B2D", // Doctor Who: Evacuation Earth
		"BH2", // Super Scribblenauts
		"BSD", // Lufia: Curse of the Sinistrals
		"BXS", // Sonic Colo(u)rs
		"BOE", // Inazuma Eleven 3: Sekai heno Chousen! The Ogre
		"BQ8", // Crafting Mama
		"BK9", // Kingdom Hearts: Re-Coded
		"BRJ", // Radiant Historia
		"IRA", // Pokemon Black Version
		"IRB", // Pokemon White Version
		"VI2", // Fire Emblem: Shin Monshou no Nazo Hikari to Kage no Eiyuu
		"BYY", // Yu-Gi-Oh 5Ds World Championship 2011: Over The Nexus
		"UZP", // Learn with Pokemon: Typing Adventure
		"B6F", // LEGO Batman 2: DC Super Heroes
		"IRE", // Pokemon Black Version 2
		"IRD", // Pokemon White Version 2
	};

	if (_isHomebrew)
	{
		return *this;
	}

	for (const char *sdktid : sdk2_list)
	{
		if (strncmp(sdktid, _gametid.c_str(), 3) == 0)
		{
			return donorSdk(2);
		}
	}

	for (const char *sdktid : sdk3_list)
	{

		if (strncmp(sdktid, _gametid.c_str(), 3) == 0)
		{
			return donorSdk(3);
		}
	}

	for (const char *sdktid : sdk4_list)
	{
		if (strncmp(sdktid, _gametid.c_str(), 3) == 0)
		{
			return donorSdk(4);
		}
	}

	if (_gametid[0] == 'V' || _sdkVersion > 0x5000000)
	{
		return donorSdk(5);
	}
	else
	{
		// TODO: If the list gets large enough, switch to bsearch().
		for (const char *sdktid : sdk5_list)
		{
			if (strncmp(sdktid, _gametid.c_str(), 3) == 0)
			{
				return donorSdk(5);
			}
		}
	}
	return donorSdk(0);
}
BootstrapConfig &BootstrapConfig::mpuSize(int mpuSize)
{
	_mpuSize = mpuSize;
	return *this;
}
BootstrapConfig &BootstrapConfig::mpuRegion(int mpuRegion)
{
	_mpuRegion = mpuRegion;
	return *this;
}
BootstrapConfig &BootstrapConfig::ceCached(bool ceCached)
{
	_ceCached = ceCached;
	return *this;
}
BootstrapConfig &BootstrapConfig::saveSize(int saveSize)
{
	_saveSize = saveSize;
	return *this;
}

BootstrapConfig &BootstrapConfig::dsiMode(int dsiMode)
{
	_dsiMode = dsiMode;
	return *this;
}

BootstrapConfig &BootstrapConfig::vramBoost(bool vramBoost)
{
	_vramBoost = vramBoost;
	return *this;
}
BootstrapConfig &BootstrapConfig::cpuBoost(bool cpuBoost)
{
	_cpuBoost = cpuBoost;
	return *this;
}

BootstrapConfig &BootstrapConfig::language(int language)
{
	_language = language;
	return *this;
}
BootstrapConfig &BootstrapConfig::saveNo(int saveNo)
{
	_saveNo = saveNo;
	return *this;
}
BootstrapConfig &BootstrapConfig::ramDiskNo(int ramDiskNo)
{
	_ramDiskNo = ramDiskNo;
	return *this;
}
BootstrapConfig &BootstrapConfig::softReset(bool softReset)
{
	_softReset = softReset;
	return *this;
}
BootstrapConfig &BootstrapConfig::soundFix(bool soundFix)
{
	_soundFix = soundFix;
	return *this;
}

BootstrapConfig &BootstrapConfig::onSaveCreated(std::function<void(void)> handler)
{
	_saveCreatedHandler = handler;
	return *this;
}

BootstrapConfig &BootstrapConfig::onConfigSaved(std::function<void(void)> handler)
{
	_configSavedHandler = handler;
	return *this;
}

BootstrapConfig &BootstrapConfig::onCheatsApplied(std::function<void(void)> handler)
{
	_cheatsAppliedHandler = handler;
	return *this;
}

BootstrapConfig &BootstrapConfig::nightlyBootstrap(bool nightlyBootstrap)
{
	_useNightlyBootstrap = nightlyBootstrap;
	return *this;
}

BootstrapConfig &BootstrapConfig::gbarBootstrap(bool gbarBootstrap)
{
	_useGbarBootstrap = gbarBootstrap;
	return *this;
}



void BootstrapConfig::createSaveFileIfNotExists()
{
	std::string savename = replaceAll(_fileName, ".nds", getSavExtension(_saveNo));
	std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
	while (!romFolderNoSlash.empty() && romFolderNoSlash[romFolderNoSlash.size()-1] == '/') {
		romFolderNoSlash.resize(romFolderNoSlash.size()-1);
	}
	std::string savepath = romFolderNoSlash+"/saves/"+savename;
	if (sdFound() && ms().secondaryDevice && ms().fcSaveOnSd) {
		savepath = replaceAll(savepath, "fat:/", "sd:/");
	}
	if (access(savepath.c_str(), F_OK) == 0)
		return;

	static const int BUFFER_SIZE = 0x1000;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));

	FILE *pFile = fopen(savepath.c_str(), "wb");
	if (pFile)
	{
		for (int i = _saveSize; i > 0; i -= BUFFER_SIZE)
		{
			fwrite(buffer, 1, sizeof(buffer), pFile);
		}
		fclose(pFile);
	}
}

void BootstrapConfig::createTmpFileIfNotExists()
{
	if (access("fat:/BTSTRP.TMP", F_OK) == 0)
		return;

	static const int BUFFER_SIZE = 0x1000;
	char buffer[BUFFER_SIZE];
	memset(buffer, 0, sizeof(buffer));

	FILE *pFile = fopen("fat:/BTSTRP.TMP", "wb");
	if (pFile)
	{
		for (u32 i = 0x40000; i > 0; i -= BUFFER_SIZE)
		{
			fwrite(buffer, 1, sizeof(buffer), pFile);
		}
		fclose(pFile);
	}
}

void BootstrapConfig::loadCheats()
{
	u32 gameCode,crc32;
	
	bool cheatsEnabled = true;
	mkdir(ms().secondaryDevice ? "fat:/_nds/nds-bootstrap" : "sd:/_nds/nds-bootstrap", 0777);
	if(CheatWnd::romData(_fullPath,gameCode,crc32))
      {
				long cheatOffset; size_t cheatSize;
        FILE* dat=fopen(SFN_CHEATS,"rb");
        if(dat)
        {
          if(CheatWnd::searchCheatData(dat,gameCode,crc32,cheatOffset,cheatSize))
          {
						CheatWnd chtwnd((256)/2,(192)/2,100,100,NULL,_fullPath);

						chtwnd.parse(_fullPath);
						chtwnd.writeCheatsToFile(chtwnd.getCheats(), SFN_CHEAT_DATA);
						FILE* cheatData=fopen(SFN_CHEAT_DATA,"rb");
						if (cheatData) {
							u32 check[2];
							fread(check, 1, 8, cheatData);
							fclose(cheatData);
							// TODO: Delete file, if above 0x8000 bytes
							if (check[1] == 0xCF000000) {
								cheatsEnabled = false;
							}
						}
          } else {
		    cheatsEnabled = false;
          }
          fclose(dat);
        } else {
		  cheatsEnabled = false;
        }
      } else {
	    cheatsEnabled = false;
      }
	  if (!cheatsEnabled) {
	    remove(SFN_CHEAT_DATA);
	  }
}

int BootstrapConfig::launch()
{
	loadCheats();
	if (ms().secondaryDevice) 
		createTmpFileIfNotExists();

	createSaveFileIfNotExists();

	if (_saveCreatedHandler)
		_saveCreatedHandler();

	std::string savename = replaceAll(_fileName, ".nds", getSavExtension(_saveNo));
	std::string ramdiskname = replaceAll(_fileName, ".nds", getImgExtension(_ramDiskNo));
	std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
	while (!romFolderNoSlash.empty() && romFolderNoSlash[romFolderNoSlash.size()-1] == '/') {
		romFolderNoSlash.resize(romFolderNoSlash.size()-1);
	}
	mkdir ((_isHomebrew && !ms().secondaryDevice) ? (romFolderNoSlash+"/ramdisks").c_str() : (romFolderNoSlash+"/saves").c_str(), 0777);
	std::string savepath = romFolderNoSlash+"/saves/"+savename;
	if (sdFound() && ms().secondaryDevice && ms().fcSaveOnSd) {
		savepath = replaceAll(savepath, "fat:/", "sd:/");
	}
	std::string ramdiskpath = romFolderNoSlash+"/ramdisks/"+ramdiskname;

	std::string bootstrapPath;

	if(_useGbarBootstrap) {
		if (_useNightlyBootstrap)
			bootstrapPath = BOOTSTRAP_NIGHTLY_GBAR;
		if (!_useNightlyBootstrap)
			bootstrapPath = BOOTSTRAP_RELEASE_GBAR;
	} else {
		if (_useNightlyBootstrap && _isHomebrew)
			bootstrapPath = BOOTSTRAP_NIGHTLY_HB;
		if (_useNightlyBootstrap && !_isHomebrew)
			bootstrapPath = BOOTSTRAP_NIGHTLY;

		if (!_useNightlyBootstrap && _isHomebrew)
			bootstrapPath = BOOTSTRAP_RELEASE_HB;
		if (!_useNightlyBootstrap && !_isHomebrew)
			bootstrapPath = BOOTSTRAP_RELEASE;

		if(access(bootstrapPath.c_str(), F_OK) != 0) {
			if (_useNightlyBootstrap && _isHomebrew)
				bootstrapPath = BOOTSTRAP_NIGHTLY_HB_FC;
			if (_useNightlyBootstrap && !_isHomebrew)
				bootstrapPath = BOOTSTRAP_NIGHTLY_FC;

			if (!_useNightlyBootstrap && _isHomebrew)
				bootstrapPath = BOOTSTRAP_RELEASE_HB_FC;
			if (!_useNightlyBootstrap && !_isHomebrew)
				bootstrapPath = BOOTSTRAP_RELEASE_FC;
		}
	}

	std::vector<char*> argarray;
	argarray.push_back(strdup(bootstrapPath.c_str()));
	argarray.at(0) = (char*)bootstrapPath.c_str();

	LoaderConfig loader(bootstrapPath, (sdFound() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC));

	loader.option("NDS-BOOTSTRAP", "NDS_PATH", _fullPath)
		.option("NDS-BOOTSTRAP", "SAV_PATH", savepath)
		.option("NDS-BOOTSTRAP", "HOMEBREW_ARG", "")
		.option("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", (_ramDiskNo >= 0 && !ms().secondaryDevice) ? ramdiskpath : "sd:/null.img")
		.option("NDS-BOOTSTRAP", "LANGUAGE", _language)
		.option("NDS-BOOTSTRAP", "BOOST_CPU", _cpuBoost)
		.option("NDS-BOOTSTRAP", "BOOST_VRAM", _vramBoost)
		.option("NDS-BOOTSTRAP", "SOUND_FIX", _soundFix)
		.option("NDS-BOOTSTRAP", "DSI_MODE", _dsiMode)
		.option("NDS-BOOTSTRAP", "DONOR_SDK_VER", _donorSdk)
		.option("NDS-BOOTSTRAP", "GAME_SOFT_RESET", _softReset)
		.option("NDS-BOOTSTRAP", "PATCH_MPU_REGION", _mpuRegion)
		.option("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", _mpuSize)
		.option("NDS-BOOTSTRAP", "CARDENGINE_CACHED", _ceCached)
		.option("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", _forceSleepPatch);

	if (_configSavedHandler)
		_configSavedHandler();
	return loader.launch(argarray.size(), (const char **)&argarray[0], (_isHomebrew ? false : true));
}
