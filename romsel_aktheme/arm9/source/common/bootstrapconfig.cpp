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
#include "saveMap.h"
#include "ROMList.h"

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
	_cpuBoost = false;
	_useGbarBootstrap = false;

	this
		->saveSize()
		.mpuSettings()
		.donorSdk();
}

BootstrapConfig::~BootstrapConfig()
{
}

BootstrapConfig &BootstrapConfig::saveSize()
{
	u32 gameTidHex = 0;
	tonccpy(&gameTidHex, &_gametid, 4);

	for (int i = 0; i < (int)sizeof(ROMList)/12; i++) {
		ROMListEntry* curentry = &ROMList[i];
		if (gameTidHex == curentry->GameCode) {
			if (curentry->SaveMemType == 0xFFFFFFFF) break;
			return saveSize(sramlen[curentry->SaveMemType]);
		}
	}

	return saveSize(0x80000);
}

BootstrapConfig &BootstrapConfig::donorSdk(int sdk)
{
	_donorSdk = sdk;
	return *this;
}

BootstrapConfig &BootstrapConfig::mpuSettings()
{
	return mpuRegion(0).mpuSize(0);
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

	int orgsavesize = 0;
    FILE* sourceFile = fopen(savepath.c_str(), "rb");
	fseek(sourceFile, 0, SEEK_END);
	orgsavesize = ftell(sourceFile);			// Get file size
	fseek(sourceFile, 0, SEEK_SET);
	fclose(sourceFile);

	if ((orgsavesize == 0 && _saveSize > 0) || (orgsavesize < _saveSize))
	{
		if (orgsavesize > 0)
		{
			fsizeincrease(savepath.c_str(), sdFound() ? "sd:/_nds/TWiLightMenu/temp.sav" : "fat:/_nds/TWiLightMenu/temp.sav", _saveSize);
		}
		else
		{
			FILE *pFile = fopen(savepath.c_str(), orgsavesize > 0 ? "r+" : "wb");
			if (pFile)
			{
				fseek(pFile, _saveSize - 1, SEEK_SET);
				fputc('\0', pFile);
				fclose(pFile);
			}
		}
	}
}

void BootstrapConfig::loadCheats()
{
	u32 gameCode,crc32;
	
	bool cheatsEnabled = true;
	mkdir(ms().secondaryDevice ? "fat:/_nds/nds-bootstrap" : "sd:/_nds/nds-bootstrap", 0777);
	if (CheatWnd::romData(_fullPath,gameCode,crc32))
      {
				long cheatOffset; size_t cheatSize;
        FILE* dat=fopen(SFN_CHEATS,"rb");
        if (dat)
        {
          if (CheatWnd::searchCheatData(dat,gameCode,crc32,cheatOffset,cheatSize))
          {
						CheatWnd chtwnd((256)/2,(192)/2,100,100,NULL,_fullPath);

						chtwnd.parse(_fullPath);
						chtwnd.writeCheatsToFile(SFN_CHEAT_DATA);
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

	if (_useGbarBootstrap) {
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

		if (access(bootstrapPath.c_str(), F_OK) != 0) {
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
		.option("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString())
		.option("NDS-BOOTSTRAP", "LANGUAGE", _language)
		.option("NDS-BOOTSTRAP", "BOOST_CPU", _cpuBoost)
		.option("NDS-BOOTSTRAP", "BOOST_VRAM", _vramBoost)
		.option("NDS-BOOTSTRAP", "DSI_MODE", _dsiMode)
		.option("NDS-BOOTSTRAP", "DONOR_SDK_VER", _donorSdk)
		.option("NDS-BOOTSTRAP", "PATCH_MPU_REGION", _mpuRegion)
		.option("NDS-BOOTSTRAP", "PATCH_MPU_SIZE", _mpuSize)
		.option("NDS-BOOTSTRAP", "CARDENGINE_CACHED", _ceCached)
		.option("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", _forceSleepPatch);

	if (_configSavedHandler)
		_configSavedHandler();
	return loader.launch(argarray.size(), (const char **)&argarray[0], (_isHomebrew ? false : true));
}
