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
#include "windows/mainwnd.h"
#include <stdio.h>
#include <nds.h>
#include <nds/arm9/dldi.h>

#include "donorMap.h"
#include "speedBumpExcludeMap.h"
#include "saveMap.h"

extern std::string getSavExtension(int number);
extern std::string getImgExtension(int number);
extern bool extention(const std::string& filename, const char* ext);

BootstrapConfig::BootstrapConfig(const std::string &fileName, const std::string &fullPath, const std::string &gametid, u32 sdkVersion, int heapShrink)
	: _fileName(fileName), _fullPath(fullPath), _gametid(gametid), _sdkVersion(sdkVersion)
{
	_donorSdk = 0;
	_mpuSize = 0;
	_mpuRegion = 0;
	_ceCached = true;
	_forceSleepPatch = (ms().forceSleepPatch
					|| (memcmp(io_dldi_data->friendlyName, "TTCARD", 6) == 0 && !sys().isRegularDS())
					|| (memcmp(io_dldi_data->friendlyName, "DSTT", 4) == 0 && !sys().isRegularDS())
					|| (memcmp(io_dldi_data->friendlyName, "DEMON", 5) == 0 && !sys().isRegularDS())
					|| (memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0 && !sys().isRegularDS()));
	_isHomebrew = _gametid.empty() || _sdkVersion == 0;
	_saveSize = 0x80000;
	_dsiMode = 0;
	_vramBoost = false;
	_language = -1;
	_saveNo = 0;
	_ramDiskNo = -1;
	_softReset = false;
	_cpuBoost = false;
	_useGbarBootstrap = false;

	this
		->saveSize()
		.softReset()
		.mpuSettings()
		.speedBumpExclude(heapShrink)
		.donorSdk();
}

BootstrapConfig::~BootstrapConfig()
{
}

BootstrapConfig &BootstrapConfig::saveSize()
{
	char gameTid3[5];
	for (int i = 0; i < 3; i++) {
		gameTid3[i] = _gametid.c_str()[i];
	}

	for (auto i : saveMap) {
		if (i.second.find(gameTid3) != i.second.cend())
			return saveSize(i.first);
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
BootstrapConfig &BootstrapConfig::speedBumpExclude(int heapShrink)
{
	if (heapShrink >= 0 && heapShrink < 2) {
		return ceCached(heapShrink);
	}

	if (!isDSiMode()) {
		for (const char *speedtid : sbeListB4DS)
		{
			if (strncmp(speedtid, _gametid.c_str(), 3) == 0)
			{
				return ceCached(false);
			}
		}
		return ceCached(true);
	}

	for (const char *speedtid : sbeList)
	{
		if (strncmp(speedtid, _gametid.c_str(), 4) == 0)
		{
			return ceCached(false);
		}
	}

	for (const char *speedtid : sbeList2)
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
	if (_isHomebrew)
	{
		return *this;
	}

	char gameTid3[5];
	for (int i = 0; i < 3; i++) {
		gameTid3[i] = _gametid.c_str()[i];
	}

	for (auto i : donorMap) {
		if (i.first == 5 && gameTid3[0] == 'V')
			return donorSdk(5);

		if (i.second.find(gameTid3) != i.second.cend())
			return donorSdk(i.first);
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

BootstrapConfig &BootstrapConfig::wideScreen(bool wideScreen)
{
	_useWideScreen = wideScreen;
	return *this;
}

BootstrapConfig &BootstrapConfig::gbarBootstrap(bool gbarBootstrap)
{
	_useGbarBootstrap = gbarBootstrap;
	return *this;
}




/**
 * Remove trailing slashes from a pathname, if present.
 * @param path Pathname to modify.
 */
void RemoveTrailingSlashes(std::string &path) {
	if (path.size() == 0) return;

	while (!path.empty() && path[path.size() - 1] == '/') {
		path.resize(path.size() - 1);
	}
}

void BootstrapConfig::createSaveFileIfNotExists()
{
	const char *typeToReplace = ".nds";
	if (extention(_fileName, ".dsi")) {
		typeToReplace = ".dsi";
	} else if (extention(_fileName, ".ids")) {
		typeToReplace = ".ids";
	} else if (extention(_fileName, ".srl")) {
		typeToReplace = ".srl";
	} else if (extention(_fileName, ".app")) {
		typeToReplace = ".app";
	}

	std::string savename = replaceAll(_fileName, typeToReplace, getSavExtension(_saveNo));
	std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];

	RemoveTrailingSlashes(romFolderNoSlash);

	std::string savepath = romFolderNoSlash + "/saves/" + savename;

	if (sdFound() && ms().secondaryDevice && ms().fcSaveOnSd) {
		savepath = replaceAll(savepath, "fat:/", "sd:/");
	}

	if (access(savepath.c_str(), F_OK) == 0)
		return;

	FILE *pFile = fopen(savepath.c_str(), "wb");
	if (pFile)
	{
		fseek(pFile, _saveSize - 1, SEEK_SET);
		fputc('\0', pFile);
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
	if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice)
		loadCheats();

	createSaveFileIfNotExists();

	if (_saveCreatedHandler)
		_saveCreatedHandler();

	bootWidescreen(_fileName.c_str(), _isHomebrew, _useWideScreen);

	const char *typeToReplace = ".nds";
	if (extention(_fileName, ".dsi")) {
		typeToReplace = ".dsi";
	} else if (extention(_fileName, ".ids")) {
		typeToReplace = ".ids";
	} else if (extention(_fileName, ".srl")) {
		typeToReplace = ".srl";
	} else if (extention(_fileName, ".app")) {
		typeToReplace = ".app";
	}

	std::string savename = replaceAll(_fileName, typeToReplace, getSavExtension(_saveNo));
	std::string ramdiskname = replaceAll(_fileName, typeToReplace, getImgExtension(_ramDiskNo));
	std::string romFolderNoSlash = ms().romfolder[ms().secondaryDevice];
	RemoveTrailingSlashes(romFolderNoSlash);
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
				bootstrapPath = (isDSiMode() ? BOOTSTRAP_NIGHTLY_FC : BOOTSTRAP_NIGHTLY_DS);

			if (!_useNightlyBootstrap && _isHomebrew)
				bootstrapPath = BOOTSTRAP_RELEASE_HB_FC;
			if (!_useNightlyBootstrap && !_isHomebrew)
				bootstrapPath = (isDSiMode() ? BOOTSTRAP_RELEASE_FC : BOOTSTRAP_RELEASE_DS);
		}
	}

	std::vector<char*> argarray;
	argarray.push_back(strdup(bootstrapPath.c_str()));
	argarray.at(0) = (char*)bootstrapPath.c_str();

	LoaderConfig loader(bootstrapPath, (sdFound() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC));

	loader.option("NDS-BOOTSTRAP", "NDS_PATH", _fullPath)
		.option("NDS-BOOTSTRAP", "SAV_PATH", savepath)
		.option("NDS-BOOTSTRAP", "AP_FIX_PATH", apFix(_fileName.c_str(), _isHomebrew))
		.option("NDS-BOOTSTRAP", "HOMEBREW_ARG", "")
		.option("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", (_ramDiskNo >= 0 && !ms().secondaryDevice) ? ramdiskpath : "sd:/null.img")
		.option("NDS-BOOTSTRAP", "LANGUAGE", _language)
		.option("NDS-BOOTSTRAP", "BOOST_CPU", _cpuBoost)
		.option("NDS-BOOTSTRAP", "BOOST_VRAM", _vramBoost)
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
