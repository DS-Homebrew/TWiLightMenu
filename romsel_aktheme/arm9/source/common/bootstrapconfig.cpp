#include "systemdetails.h"
#include "bootstrapconfig.h"
#include "dsimenusettings.h"
#include "filecopy.h"
#include "flashcard.h"
#include "loaderconfig.h"
#include "tool/stringtool.h"
#include <stdio.h>

extern std::string getSavExtension(int number);

BootstrapConfig::BootstrapConfig(const std::string &fileName, const std::string &fullPath, const std::string &gametid, u32 sdkVersion)
	: _fileName(fileName), _fullPath(fullPath), _gametid(gametid), _sdkVersion(sdkVersion)
{
	_donorSdk = 0;
	_mpuSize = 0;
	_mpuRegion = 0;
	_forceSleepPatch = (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS()) ? true : false;
	_isHomebrew = _gametid.empty() || _sdkVersion == 0;
	_saveSize = 0x80000;
	_dsiMode = 0;
	_vramBoost = false;
	_language = -1;
	_saveNo = 0;
	_softReset = false;
	_soundFix = false;
	_cpuBoost = false;

	this
		->saveSize()
		.softReset()
		.mpuSettings()
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

	if (strncmp("UOR", _gametid.c_str(), 3) == 0 || strncmp("BKI", _gametid.c_str(), 3) == 0)
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



void BootstrapConfig::createSaveFileIfNotExists()
{
	std::string savename = replaceAll(_fileName, ".nds", getSavExtension(_saveNo));
	std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
	while (!romFolderNoSlash.empty() && romFolderNoSlash[romFolderNoSlash.size()-1] == '/') {
		romFolderNoSlash.resize(romFolderNoSlash.size()-1);
	}
	std::string savepath = romFolderNoSlash+"/saves/"+savename;
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

int BootstrapConfig::launch()
{
	if (ms().secondaryDevice) 
		createTmpFileIfNotExists();

	createSaveFileIfNotExists();

	if (_saveCreatedHandler)
		_saveCreatedHandler();

	std::string savename = replaceAll(_fileName, ".nds", getSavExtension(_saveNo));
	std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
	while (!romFolderNoSlash.empty() && romFolderNoSlash[romFolderNoSlash.size()-1] == '/') {
		romFolderNoSlash.resize(romFolderNoSlash.size()-1);
	}
	mkdir ("saves", 0777);
	std::string savepath = romFolderNoSlash+"/saves/"+savename;

	if (sdFound() && ms().secondaryDevice) {
		fcopy(BOOTSTRAP_INI, BOOTSTRAP_INI_FC);		// Sync nds-bootstrap SD settings to flashcard
	}

	std::string bootstrapPath;
	if (ms().secondaryDevice) {
		if (_useNightlyBootstrap && _isHomebrew)
			bootstrapPath = BOOTSTRAP_NIGHTLY_HB_FC;
		if (_useNightlyBootstrap && !_isHomebrew)
			bootstrapPath = BOOTSTRAP_NIGHTLY_FC;

		if (!_useNightlyBootstrap && _isHomebrew)
			bootstrapPath = BOOTSTRAP_RELEASE_HB_FC;
		if (!_useNightlyBootstrap && !_isHomebrew)
			bootstrapPath = BOOTSTRAP_RELEASE_FC;
	} else {
		if (_useNightlyBootstrap && _isHomebrew)
			bootstrapPath = BOOTSTRAP_NIGHTLY_HB;
		if (_useNightlyBootstrap && !_isHomebrew)
			bootstrapPath = BOOTSTRAP_NIGHTLY;

		if (!_useNightlyBootstrap && _isHomebrew)
			bootstrapPath = BOOTSTRAP_RELEASE_HB;
		if (!_useNightlyBootstrap && !_isHomebrew)
			bootstrapPath = BOOTSTRAP_RELEASE;
	}

	LoaderConfig loader(bootstrapPath, (ms().secondaryDevice ? BOOTSTRAP_INI_FC : BOOTSTRAP_INI));

	loader.option("NDS-BOOTSTRAP", "NDS_PATH", _fullPath)
		.option("NDS-BOOTSTRAP", "SAV_PATH", savepath)
		.option("NDS-BOOTSTRAP", "LANGUAGE", _language)
		.option("NDS-BOOTSTRAP", "BOOST_CPU", _cpuBoost)
		.option("NDS-BOOTSTRAP", "BOOST_VRAM", _vramBoost)
		.option("NDS-BOOTSTRAP", "SOUND_FIX", _soundFix)
		.option("NDS-BOOTSTRAP", "DSI_MODE", _dsiMode)
		.option("NDS-BOOTSTRAP", "DONOR_SDK_VER", _donorSdk)
		.option("NDS-BOOTSTRAP", "GAME_SOFT_RESET", _softReset)
		.option("NDS-BOOTSTRAP", "PATCH_MPU_REGION", _mpuRegion)
		.option("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", _mpuSize)
		.option("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", _forceSleepPatch);

	if (_configSavedHandler)
		_configSavedHandler();
	return loader.launch();
}