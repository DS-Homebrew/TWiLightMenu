#include "systemdetails.h"
#include "bootstrapconfig.h"
#include "widescreenconfig.h"
#include "dsimenusettings.h"
#include "filecopy.h"
#include "flashcard.h"
#include "loaderconfig.h"
#include "tool/stringtool.h"
#include <stdio.h>
#include <nds.h>
#include <nds/arm9/dldi.h>

#include "donorMap.h"
#include "mpuMap.h"
#include "speedBumpExcludeMap.h"
#include "incompatibleGameMap.h"
#include "saveMap.h"

extern std::string getSavExtension(int number);
extern std::string getImgExtension(int number);
extern bool extention(const std::string& filename, const char* ext);

BootstrapConfig::BootstrapConfig(const std::string &fileName, const std::string &fullPath, const u8* gametid, u32 sdkVersion, u16 headerCrc16, int heapShrink)
	: _fileName(fileName), _fullPath(fullPath), _gametid(std::string((char *)gametid, 4)), _sdkVersion(sdkVersion), _headerCrc16(headerCrc16)
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
	_cheatData = std::string();

	this
		->saveSize()
		.mpuSettings()
		.speedBumpExclude(heapShrink)
		.donorSdk();
	_apFix = getAPFix();
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

BootstrapConfig &BootstrapConfig::donorSdk(int sdk)
{
	_donorSdk = sdk;
	return *this;
}

BootstrapConfig &BootstrapConfig::mpuSettings()
{
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
	if (!isDSiMode() || (heapShrink >= 0 && heapShrink < 2)) {
		return ceCached(heapShrink);
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

BootstrapConfig &BootstrapConfig::cheatData(const std::string& cheatData)
{
	_cheatData = cheatData;
	return *this;
}

BootstrapConfig &BootstrapConfig::onSaveCreated(std::function<void(void)> handler)
{
	_saveCreatedHandler = handler;
	return *this;
}

BootstrapConfig &BootstrapConfig::onWidescreenFailed(std::function<void(std::string)> handler)
{
	_widescreenFailedHandler = handler;
	return *this;
}

BootstrapConfig &BootstrapConfig::onBeforeSaveCreate(std::function<void(void)> handler)
{
	_saveBeforeCreatedHandler = handler;
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

off_t fsize(const char *path)
{
	struct stat st;
	stat(path, &st);
	return st.st_size;
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

std::string BootstrapConfig::createSaveFileIfNotExists() {
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
	std::string romFolderNoSlash = ms().getCurrentRomFolder();

	RemoveTrailingSlashes(romFolderNoSlash);
	mkdir((romFolderNoSlash + "/saves/").c_str(), 0777);
	std::string savepath = romFolderNoSlash + "/saves/" + savename;

	if (sdFound() && ms().secondaryDevice && ms().fcSaveOnSd) {
		savepath = replaceAll(savepath, "fat:/", "sd:/");
	}

	int orgsavesize = fsize(savepath.c_str());;
	bool saveSizeFixNeeded = false;

	// TODO: If the list gets large enough, switch to bsearch().
	for (unsigned int i = 0; i < sizeof(saveSizeFixList) / sizeof(saveSizeFixList[0]); i++) {
		if (memcmp(_gametid.c_str(), saveSizeFixList[i], 3) == 0) {
			// Found a match.
			saveSizeFixNeeded = true;
			break;
		}
	}

	if ((orgsavesize == 0 && _saveSize > 0) || (orgsavesize < _saveSize && saveSizeFixNeeded))
	{
		if (orgsavesize > 0)
		{
			fsizeincrease(savepath.c_str(), sdFound() ? "sd:/_nds/TWiLightMenu/temp.sav" : "fat:/_nds/TWiLightMenu/temp.sav", _saveSize);
		}
		else
		{
			FILE *pFile = fopen(savepath.c_str(), "wb");
			if (pFile)
			{
				fseek(pFile, _saveSize - 1, SEEK_SET);
				fputc('\0', pFile);
				fclose(pFile);
			}
		}
	}

	return savepath;
}

void BootstrapConfig::loadCheats() {
	mkdir("/_nds", 0777);
	mkdir("/_nds/nds-bootstrap", 0777);

	remove(CHEATS_CHEAT_DATA);
	if (_cheatData.empty()) return;
	
	FILE* cheatFile = fopen(CHEATS_CHEAT_DATA, "wb+");
	if (!cheatFile) {
		fclose(cheatFile);
		return;
	}

	std::string cheatStr = _cheatData;
	uint32_t value;

	while (1) {
		std::string current_cheat = cheatStr.substr(0, cheatStr.find(" "));
		cheatStr = cheatStr.substr(cheatStr.find(" ") + 1);
    	value = strtol(current_cheat.c_str(), NULL, 16);
  		fwrite(&value, sizeof(value), 1, cheatFile);
    	if((int)cheatStr.find(" ") == -1) break;
	}
	fwrite("\0\0\0\xCF", 4, 1, cheatFile);
	fflush(cheatFile);
	fseek(cheatFile, 0, SEEK_SET);
	u32 check[2];
	fread(check, 1, 8, cheatFile);
	fclose(cheatFile);
	if (check[1] == 0xCF000000) {
		// cheats disabled
		remove(CHEATS_CHEAT_DATA);
	}
}

int BootstrapConfig::launch()
{
	if (_saveBeforeCreatedHandler) {
		_saveBeforeCreatedHandler();
	}
	
	std::string savepath = createSaveFileIfNotExists();

	if (_saveCreatedHandler)
		_saveCreatedHandler();

	if ((isDSiMode() && ms().useBootstrap) || !ms().secondaryDevice) {
		loadCheats();
		if (_cheatsAppliedHandler) {
			_cheatsAppliedHandler();
		}
	}

	WidescreenConfig widescreen(_fileName);

	std::string error = widescreen
		.isHomebrew(_isHomebrew)
		.gamePatch(_gametid.c_str(), _headerCrc16)
		.enable(_useWideScreen)
		.apply();

	if (!error.empty() && _widescreenFailedHandler) {
		_widescreenFailedHandler(error);
	}

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

	std::string ramdiskname = replaceAll(_fileName, typeToReplace, getImgExtension(_ramDiskNo));
	std::string romFolderNoSlash = ms().getCurrentRomFolder();
	RemoveTrailingSlashes(romFolderNoSlash);

	mkdir((_isHomebrew && !ms().secondaryDevice) ? (romFolderNoSlash+"/ramdisks").c_str() : (romFolderNoSlash+"/saves").c_str(), 0777);

	std::string ramdiskpath = romFolderNoSlash+"/ramdisks/"+ramdiskname;

	std::string bootstrapPath;

	if(_useGbarBootstrap) {
		if (_useNightlyBootstrap)
			bootstrapPath = BOOTSTRAP_NIGHTLY_GBAR;
		if (!_useNightlyBootstrap)
			bootstrapPath = BOOTSTRAP_RELEASE_GBAR;
	} else if (isDSiMode()) {
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
	} else {
		// b4ds path
		if (_useNightlyBootstrap) {
			bootstrapPath = BOOTSTRAP_NIGHTLY_DS;
		} else {
			bootstrapPath = BOOTSTRAP_RELEASE_DS;
		}
	}

	std::vector<char*> argarray;
	argarray.push_back(strdup(bootstrapPath.c_str()));
	argarray.at(0) = (char*)bootstrapPath.c_str();

	LoaderConfig loader(bootstrapPath, (sdFound() ? BOOTSTRAP_INI : BOOTSTRAP_INI_FC));

	loader.option("NDS-BOOTSTRAP", "NDS_PATH", _fullPath)
		.option("NDS-BOOTSTRAP", "SAV_PATH", savepath)
		.option("NDS-BOOTSTRAP", "AP_FIX_PATH", _apFix)
		.option("NDS-BOOTSTRAP", "HOMEBREW_ARG", "")
		.option("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", (_ramDiskNo >= 0 && !ms().secondaryDevice) ? ramdiskpath : "sd:/null.img")
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

std::string BootstrapConfig::getAPFix()
{
	remove("fat:/_nds/nds-bootstrap/apFix.ips");

	if (_isHomebrew) {
		return "";
	}

	bool ipsFound = false;
	char ipsPath[256];
	snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/apfix/%s.ips", sdFound() ? "sd" : "fat", _fileName.c_str());
	ipsFound = (access(ipsPath, F_OK) == 0);

	if (!ipsFound) {
		snprintf(ipsPath, sizeof(ipsPath), "%s:/_nds/TWiLightMenu/apfix/%s-%X.ips", sdFound() ? "sd" : "fat", _gametid.c_str(), _headerCrc16);
		nocashWrite(ipsPath, sizeof(ipsPath));
		ipsFound = (access(ipsPath, F_OK) == 0);
	}

	if (ipsFound) {
		if (ms().secondaryDevice && sdFound()) {
			mkdir("fat:/_nds", 0777);
			mkdir("fat:/_nds/nds-bootstrap", 0777);
			fcopy(ipsPath, "fat:/_nds/nds-bootstrap/apFix.ips");
			return "fat:/_nds/nds-bootstrap/apFix.ips";
		}
		return std::string(ipsPath);
	}

	return "";
}

bool BootstrapConfig::checkCompatibility() {
	if (!isDSiMode()) {
		// TODO: If the list gets large enough, switch to bsearch().
		for (unsigned int i = 0; i < sizeof(incompatibleGameListB4DS)/sizeof(incompatibleGameListB4DS[0]); i++) {
			if (memcmp(_gametid.c_str(), incompatibleGameListB4DS[i], 3) == 0) {
				return false;
			}
		}
	} 
	for (unsigned int i = 0; i < sizeof(incompatibleGameList)/sizeof(incompatibleGameList[0]); i++) {
		if (memcmp(_gametid.c_str(), incompatibleGameList[i], 3) == 0) {
			// Found match
			return false;
		}
	}
	return true;
}

APFixType BootstrapConfig::hasAPFix() {
	nocashWrite(_apFix.c_str(), _apFix.length());
	if (!_apFix.empty()) return APFixType::EHasIPS; // we have a known AP fix ready.
	nocashWrite(_gametid.c_str(), 4);
	// Check for SDK4-5 ROMs that don't have AP measures.
	if ((_gametid.rfind("AZLJ", 0) == 0)	// Girls Mode (JAP version of Style Savvy)
		 || (_gametid.rfind("YEEJ", 0) == 0)	// Inazuma Eleven (Japan)
		 || (_gametid.rfind("CNSX", 0) == 0)	// Naruto Shippuden: Naruto vs Sasuke (Europe)
		 || (_gametid.rfind("BH2J", 0) == 0))	// Super Scribblenauts (Japan)
	{
		return APFixType::ENone;
	}

	if ((_gametid.rfind("VETP", 0) == 0)	// 1000 Cooking Recipes from Elle a Table (Europe)
		 || (_gametid.rfind("CQQP", 0) == 0)	// AFL Mascot Manor (Australia)
		 || (_gametid.rfind("CA5E", 0) == 0)	// Again: Interactive Crime Novel (USA)
		 || (_gametid.rfind("TAKJ", 0) == 0)	// All Kamen Rider: Rider Generation (Japan)
		 || (_gametid.rfind("BKCE", 0) == 0)	// America's Test Kitchen: Let's Get Cooking (USA)
		 || (_gametid.rfind("A3PJ", 0) == 0)	// Anpanman to Touch de Waku Waku Training (Japan)
		 || (_gametid.rfind("B2AK", 0) == 0)	// Aranuri: Badachingudeulkkwa hamkke Mandeuneun Sesang (Korea)
		 || (_gametid.rfind("BB4J", 0) == 0)	// Battle Spirits Digital Starter (Japan)
		 || (_gametid.rfind("CYJJ", 0) == 0)	// Blood of Bahamut (Japan)
		 || (_gametid.rfind("TBSJ", 0) == 0)	// Byoutai Seiri DS: Image Dekiru! Shikkan, Shoujou to Care (Japan)
		 || (_gametid.rfind("C5YJ", 0) == 0)	// Chocobo to Mahou no Ehon: Majo to Shoujo to 5-nin no Yuusha (Japan)
		 || (_gametid.rfind("C6HK", 0) == 0)	// Chuldong! Rescue Force DS (Korea)
		 || (_gametid.rfind("CCTJ", 0) == 0)	// Cid to Chocobo no Fushigi na Dungeon: Toki Wasure no Meikyuu DS+ (Japan)
		 || (_gametid.rfind("CLPD", 0) == 0)	// Club Penguin: Elite Penguin Force (Germany)
		 || (_gametid.rfind("BQ6J", 0) == 0)	// Cocoro no Cocoron (Japan)
		 || (_gametid.rfind("BQIJ", 0) == 0)	// Cookin' Idol I! My! Mine!: Game de Hirameki! Kirameki! Cooking (Japan)
		 || (_gametid.rfind("B3CJ", 0) == 0)	// Cooking Mama 3 (Japan)
		 || (_gametid.rfind("TMCP", 0) == 0)	// Cooking Mama World: Combo Pack: Volume 1 (Europe)
		 || (_gametid.rfind("TMDP", 0) == 0)	// Cooking Mama World: Combo Pack: Volume 2 (Europe)
		 || (_gametid.rfind("BJ8P", 0) == 0)	// Cooking Mama World: Hobbies & Fun (Europe)
		 || (_gametid.rfind("VCPJ", 0) == 0)	// Cosmetick Paradise: Kirei no Mahou (Japan)
		 || (_gametid.rfind("VCTJ", 0) == 0)	// Cosmetick Paradise: Princess Life (Japan)
		 || (_gametid.rfind("BQBJ", 0) == 0)	// Crayon Shin-chan: Obaka Dainin Den: Susume! Kasukabe Ninja Tai! (Japan)
		 || (_gametid.rfind("BUCJ", 0) == 0)	// Crayon Shin-chan: Shock Gahn!: Densetsu o Yobu Omake Daiketsusen!! (Japan)
		 || (_gametid.rfind("BDNJ", 0) == 0)	// Cross Treasures (Japan)
		 || (_gametid.rfind("TPGJ", 0) == 0)	// Dengeki Gakuen RPG: Cross of Venus Special (Japan)
		 || (_gametid.rfind("BLEJ", 0) == 0)	// Digimon Story: Lost Evolution (Japan)
		 || (_gametid.rfind("TBFJ", 0) == 0)	// Digimon Story: Super Xros Wars: Blue (Japan)
		 || (_gametid.rfind("TLTJ", 0) == 0)	// Digimon Story: Super Xros Wars: Red (Japan)
		 || (_gametid.rfind("BVIJ", 0) == 0)	// Dokonjou Shougakusei Bon Biita: Hadaka no Choujou Ketsusen!!: Biita vs Dokuro Dei! (Japan)
		 || (_gametid.rfind("TDBJ", 0) == 0)	// Dragon Ball Kai: Ultimate Butou Den (Japan)
		 || (_gametid.rfind("B2JJ", 0) == 0)	// Dragon Quest Monsters: Joker 2: Professional (Japan)
		 || (_gametid.rfind("YVKK", 0) == 0)	// DS Vitamin: Widaehan Bapsang: Malhaneun! Geongangyori Giljabi (Korea)
		 || (_gametid.rfind("B3LJ", 0) == 0)	// Eigo de Tabisuru: Little Charo (Japan)
		 || (_gametid.rfind("VL3J", 0) == 0)	// Elminage II: Sousei no Megami to Unmei no Daichi: DS Remix (Japan)
		 || (_gametid.rfind("THMJ", 0) == 0)	// FabStyle (Japan)
		 || (_gametid.rfind("VI2J", 0) == 0)	// Fire Emblem: Shin Monshou no Nazo: Hikari to Kage no Eiyuu (Japan)
		 || (_gametid.rfind("BFPJ", 0) == 0)	// Fresh PreCure!: Asobi Collection (Japan)
		 || (_gametid.rfind("B4FJ", 0) == 0)	// Fushigi no Dungeon: Fuurai no Shiren 4: Kami no Hitomi to Akuma no Heso (Japan)
		 || (_gametid.rfind("B5FJ", 0) == 0)	// Fushigi no Dungeon: Fuurai no Shiren 5: Fortune Tower to Unmei no Dice (Japan)
		 || (_gametid.rfind("BG3J", 0) == 0)	// G.G Series Collection+ (Japan)
		 || (_gametid.rfind("BRQJ", 0) == 0)	// Gendai Daisenryaku DS: Isshoku Sokuhatsu, Gunji Balance Houkai (Japan)
		 || (_gametid.rfind("VMMJ", 0) == 0)	// Gokujou!! Mecha Mote Iinchou: MM My Best Friend! (Japan)
		 || (_gametid.rfind("BM7J", 0) == 0)	// Gokujou!! Mecha Mote Iinchou: MM Town de Miracle Change! (Japan)
		 || (_gametid.rfind("BXOJ", 0) == 0)	// Gyakuten Kenji 2 (Japan)
		 || (_gametid.rfind("BQFJ", 0) == 0)	// HeartCatch PreCure!: Oshare Collection (Japan)
		 || (_gametid.rfind("AWIK", 0) == 0)	// Hotel Duskui Bimil (Korea)
		 || (_gametid.rfind("YHGJ", 0) == 0)	// Houkago Shounen (Japan)
		 || (_gametid.rfind("BRYJ", 0) == 0)	// Hudson x GReeeeN: Live! DeeeeS! (Japan)
		 || (_gametid.rfind("YG4K", 0) == 0)	// Hwansangsuhojeon: Tierkreis (Korea)
		 || (_gametid.rfind("BZ2J", 0) == 0)	// Imasugu Tsukaeru Mamechishiki: Quiz Zatsugaku-ou DS (Japan)
		 || (_gametid.rfind("BEZJ", 0) == 0)	// Inazuma Eleven 3: Sekai e no Chousen!!: Bomber (Japan)
		 || (_gametid.rfind("BE8J", 0) == 0)	// Inazuma Eleven 3: Sekai e no Chousen!!: Spark (Japan)
		 || (_gametid.rfind("BOEJ", 0) == 0)	// Inazuma Eleven 3: Sekai e no Chousen!!: The Ogre (Japan)
		 || (_gametid.rfind("BJKJ", 0) == 0)	// Ippan Zaidan Houjin Nihon Kanji Shuujukudo Kentei Kikou Kounin: Kanjukuken DS (Japan)
		 || (_gametid.rfind("BIMJ", 0) == 0)	// Iron Master: The Legendary Blacksmith (Japan)
		 || (_gametid.rfind("CDOK", 0) == 0)	// Iron Master: Wanggugui Yusangwa Segaeui Yeolsoe (Korea)
		 || (_gametid.rfind("YROK", 0) == 0)	// Isanghan Naraui Princess (Korea)
		 || (_gametid.rfind("BRGJ", 0) == 0)	// Ishin no Arashi: Shippuu Ryouma Den (Japan)
		 || (_gametid.rfind("UXBP", 0) == 0)	// Jam with the Band (Europe)
		 || (_gametid.rfind("YEOK", 0) == 0)	// Jeoldaepiryo: Yeongsugeo 1000 DS (Korea)
		 || (_gametid.rfind("YE9K", 0) == 0)	// Jeoldaeuwi: Yeongdaneo 1900 DS (Korea)
		 || (_gametid.rfind("B2OK", 0) == 0)	// Jeongukmin Model Audition Superstar DS (Korea)
		 || (_gametid.rfind("YRCK", 0) == 0)	// Jjangguneun Monmallyeo: Cinemaland Chalkak Chalkak Daesodong! (Korea)
		 || (_gametid.rfind("CL4K", 0) == 0)	// Jjangguneun Monmallyeo: Mallangmallang Gomuchalheuk Daebyeonsin! (Korea)
		 || (_gametid.rfind("BOBJ", 0) == 0)	// Kaidan Restaurant: Ura Menu 100-sen (Japan)
		 || (_gametid.rfind("TK2J", 0) == 0)	// Kaidan Restaurant: Zoku! Shin Menu 100-sen (Japan)
		 || (_gametid.rfind("BA7J", 0) == 0)	// Kaibou Seirigaku DS: Touch de Hirogaru! Jintai no Kouzou to Kinou (Japan)
		 || (_gametid.rfind("BKXJ", 0) == 0)	// Kaijuu Busters (Japan)
		 || (_gametid.rfind("BYVJ", 0) == 0)	// Kaijuu Busters Powered (Japan)
		 || (_gametid.rfind("TGKJ", 0) == 0)	// Kaizoku Sentai Gokaiger: Atsumete Henshin! 35 Sentai! (Japan)
		 || (_gametid.rfind("B8RJ", 0) == 0)	// Kamen Rider Battle: GanbaRide: Card Battle Taisen (Japan)
		 || (_gametid.rfind("BKHJ", 0) == 0)	// Kamonohashikamo.: Aimai Seikatsu no Susume (Japan)
		 || (_gametid.rfind("BKPJ", 0) == 0)	// Kanshuu: Shuukan Pro Wrestling: Pro Wrestling Kentei DS (Japan)
		 || (_gametid.rfind("BEKJ", 0) == 0)	// Kanzen Taiou Saishin Kako Mondai: Nijishiken Taisaku: Eiken Kanzenban (Japan)
		 || (_gametid.rfind("BQJJ", 0) == 0)	// Kawaii Koneko DS 3 (Japan)
		 || (_gametid.rfind("BKKJ", 0) == 0)	// Keroro RPG: Kishi to Musha to Densetsu no Kaizoku (Japan)
		 || (_gametid.rfind("BKSJ", 0) == 0)	// Keshikasu-kun: Battle Kasu-tival (Japan)
		 || (_gametid.rfind("BKTJ", 0) == 0)	// Kimi ni Todoke: Sodateru Omoi (Japan)
		 || (_gametid.rfind("TK9J", 0) == 0)	// Kimi ni Todoke: Special (Japan)
		 || (_gametid.rfind("TKTJ", 0) == 0)	// Kimi ni Todoke: Tsutaeru Kimochi (Japan)
		 || (_gametid.rfind("CKDJ", 0) == 0)	// Kindaichi Shounen no Jikenbo: Akuma no Satsujin Koukai (Japan)
		 || (_gametid.rfind("VCGJ", 0) == 0)	// Kirakira Rhythm Collection (Japan)
		 || (_gametid.rfind("BCKJ", 0) == 0)	// Kochira Katsushika Ku Kameari Kouen Mae Hashutsujo: Kateba Tengoku! Makereba Jigoku!: Ryoutsu-ryuu Ikkakusenkin Daisakusen! (Japan)
		 || (_gametid.rfind("BCXJ", 0) == 0)	// Kodawari Saihai Simulation: Ochanoma Pro Yakyuu DS: 2010 Nendo Ban (Japan)
		 || (_gametid.rfind("VKPJ", 0) == 0)	// Korg DS-10+ Synthesizer Limited Edition (Japan)
		 || (_gametid.rfind("BZMJ", 0) == 0)	// Korg M01 Music Workstation (Japan)
		 || (_gametid.rfind("BTAJ", 0) == 0)	// Lina no Atelier: Strahl no Renkinjutsushi (Japan)
		 || (_gametid.rfind("BCDK", 0) == 0)	// Live-On Card Live-R DS (Korea)
		 || (_gametid.rfind("VLIP", 0) == 0)	// Lost Identities (Europe)
		 || (_gametid.rfind("BOXJ", 0) == 0)	// Love Plus+ (Japan)
		 || (_gametid.rfind("BL3J", 0) == 0)	// Lupin Sansei: Shijou Saidai no Zunousen (Japan)
		 || (_gametid.rfind("YNOK", 0) == 0)	// Mabeopcheonjamun DS (Korea)
		 || (_gametid.rfind("BCJK", 0) == 0)	// Mabeopcheonjamun DS 2 (Korea)
		 || (_gametid.rfind("ANMK", 0) == 0)	// Maeilmaeil Deoukdeo!: DS Dunoe Training (Korea)
		 || (_gametid.rfind("TCYE", 0) == 0)	// Mama's Combo Pack: Volume 1 (USA)
		 || (_gametid.rfind("TCZE", 0) == 0)	// Mama's Combo Pack: Volume 2 (USA)
		 || (_gametid.rfind("ANMK", 0) == 0)	// Marie-Antoinette and the American War of Independence: Episode 1: The Brotherhood of the Wolf (Europe)
		 || (_gametid.rfind("BA5P", 0) == 0)	// Mario & Luigi RPG: Siganui Partner (Korea)
		 || (_gametid.rfind("C6OJ", 0) == 0)	// Medarot DS: Kabuto Ver. (Japan)
		 || (_gametid.rfind("BQWJ", 0) == 0)	// Medarot DS: Kuwagata Ver. (Japan)
		 || (_gametid.rfind("BBJJ", 0) == 0)	// Metal Fight Beyblade: Baku Shin Susanoo Shuurai! (Japan)
		 || (_gametid.rfind("TKNJ", 0) == 0)	// Meitantei Conan: Aoki Houseki no Rondo (Japan)
		 || (_gametid.rfind("TMKJ", 0) == 0)	// Meitantei Conan: Kako Kara no Zensou Kyoku (Japan)
		 || (_gametid.rfind("TMXJ", 0) == 0)	// Metal Max 2: Reloaded (Japan)
		 || (_gametid.rfind("C34J", 0) == 0)	// Mini Yonku DS (Japan)
		 || (_gametid.rfind("BWCJ", 0) == 0)	// Minna no Conveni (Japan)
		 || (_gametid.rfind("BQUJ", 0) == 0)	// Minna no Suizokukan (Japan)
		 || (_gametid.rfind("BQVJ", 0) == 0)	// Minna to Kimi no Piramekino! (Japan)
		 || (_gametid.rfind("B2WJ", 0) == 0)	// Moe Moe 2-ji Taisen(ryaku) Two: Yamato Nadeshiko (Japan)
		 || (_gametid.rfind("BWRJ", 0) == 0)	// Momotarou Dentetsu: World (Japan)
		 || (_gametid.rfind("CZZK", 0) == 0)	// Monmallineun 3-gongjuwa Hamkkehaneun: Geurimyeonsang Yeongdaneo Amgibeop (Korea)
		 || (_gametid.rfind("B3IJ", 0) == 0)	// Motto! Stitch! DS: Rhythm de Rakugaki Daisakusen (Japan)
		 || (_gametid.rfind("C6FJ", 0) == 0)	// Mugen no Frontier Exceed: Super Robot Taisen OG Saga (Japan)
		 || (_gametid.rfind("B74J", 0) == 0)	// Nanashi no Geemu Me (Japan)
		 || (_gametid.rfind("TNRJ", 0) == 0)	// Nora to Toki no Koubou: Kiri no Mori no Majo (Japan)
		 || (_gametid.rfind("YNRK", 0) == 0)	// Naruto Jilpungjeon: Daenantu! Geurimja Bunsinsul (Korea)
		 || (_gametid.rfind("BKJJ", 0) == 0)	// Nazotte Oboeru: Otona no Kanji Renshuu: Kaiteiban (Japan)
		 || (_gametid.rfind("BPUJ", 0) == 0)	// Nettou! Powerful Koushien (Japan)
		 || (_gametid.rfind("TJ7J", 0) == 0)	// New Horizon: English Course 3 (Japan)
		 || (_gametid.rfind("TJ8J", 0) == 0)	// New Horizon: English Course 2 (Japan)
		 || (_gametid.rfind("TJ9J", 0) == 0)	// New Horizon: English Course 1 (Japan)
		 || (_gametid.rfind("BETJ", 0) == 0)	// Nihon Keizai Shinbunsha Kanshuu: Shiranai Mama dewa Son wo Suru: 'Mono ya Okane no Shikumi' DS (Japan)
		 || (_gametid.rfind("YCUP", 0) == 0)	// Nintendo Presents: Crossword Collection (Europe)
		 || (_gametid.rfind("B2KJ", 0) == 0)	// Ni no Kuni: Shikkoku no Madoushi (Japan)
		 || (_gametid.rfind("BNCJ", 0) == 0)	// Nodame Cantabile: Tanoshii Ongaku no Jikan Desu (Japan)
		 || (_gametid.rfind("CQKP", 0) == 0)	// NRL Mascot Mania (Australia)
		 || (_gametid.rfind("BO4J", 0) == 0)	// Ochaken no Heya DS 4 (Japan)
		 || (_gametid.rfind("BOYJ", 0) == 0)	// Odoru Daisousa-sen: The Game: Sensuikan ni Sennyuu Seyo! (Japan)
		 || (_gametid.rfind("B62J", 0) == 0)	// Okaeri! Chibi-Robo!: Happy Rich Oosouji! (Japan)
		 || (_gametid.rfind("TGBJ", 0) == 0)	// One Piece Gigant Battle 2: Shin Sekai (Japan)
		 || (_gametid.rfind("BOKJ", 0) == 0)	// Ookami to Koushinryou: Umi o Wataru Kaze (Japan)
		 || (_gametid.rfind("TKDJ", 0) == 0)	// Ore-Sama Kingdom: Koi mo Manga mo Debut o Mezase! Doki Doki Love Lesson (Japan)
		 || (_gametid.rfind("TFTJ", 0) == 0)	// Original Story from Fairy Tail: Gekitotsu! Kardia Daiseidou (Japan)
		 || (_gametid.rfind("BHQJ", 0) == 0)	// Otona no Renai Shousetsu: DS Harlequin Selection (Japan)
		 || (_gametid.rfind("BIPJ", 0) == 0)	// Pen1 Grand Prix: Penguin no Mondai Special (Japan)
		 || (_gametid.rfind("BO9J", 0) == 0)	// Penguin no Mondai: The World (Japan)
		 || (_gametid.rfind("B42J", 0) == 0)	// Pet Shop Monogatari DS 2 (Japan)
		 || (_gametid.rfind("BVGE", 0) == 0)	// Petz: Bunnyz Bunch (USA)
		 || (_gametid.rfind("BLLE", 0) == 0)	// Petz: Catz Playground (USA)
		 || (_gametid.rfind("BUFE", 0) == 0)	// Petz: Puppyz & Kittenz (USA)
		 || (_gametid.rfind("VFBE", 0) == 0)	// Petz Fantasy: Moonlight Magic (USA)
		 || (_gametid.rfind("VTPV", 0) == 0)	// Phineas and Ferb: 2 Disney Games (Europe)
		 || (_gametid.rfind("B5VE", 0) == 0)	// Phineas and Ferb: Across the 2nd Dimension (USA)
		 || (_gametid.rfind("YFTK", 0) == 0)	// Pokemon Bulgasaui Dungeon: Siganui Tamheomdae (Korea)
		 || (_gametid.rfind("YFYK", 0) == 0)	// Pokemon Bulgasaui Dungeon: Eodumui Tamheomdae (Korea)
		 || (_gametid.rfind("BPPJ", 0) == 0)	// PostPet DS: Yumemiru Momo to Fushigi no Pen (Japan)
		 || (_gametid.rfind("BONJ", 0) == 0)	// Powerful Golf (Japan)
		 || (_gametid.rfind("VPTJ", 0) == 0)	// Power Pro Kun Pocket 12 (Japan)
		 || (_gametid.rfind("VPLJ", 0) == 0)	// Power Pro Kun Pocket 13 (Japan)
		 || (_gametid.rfind("VP4J", 0) == 0)	// Power Pro Kun Pocket 14 (Japan)
		 || (_gametid.rfind("B2YK", 0) == 0)	// Ppiyodamari DS (Korea)
		 || (_gametid.rfind("B4NK", 0) == 0)	// Princess Angel: Baeguiui Cheonsa (Korea)
		 || (_gametid.rfind("C4WK", 0) == 0)	// Princess Bakery (Korea)
		 || (_gametid.rfind("CP4K", 0) == 0)	// Princess Maker 4: Special Edition (Korea)
		 || (_gametid.rfind("C29J", 0) == 0)	// Pro Yakyuu Famista DS 2009 (Japan)
		 || (_gametid.rfind("BF2J", 0) == 0)	// Pro Yakyuu Famista DS 2010 (Japan)
		 || (_gametid.rfind("B89J", 0) == 0)	// Pro Yakyuu Team o Tsukurou! 2 (Japan)
		 || (_gametid.rfind("BU9J", 0) == 0)	// Pucca: Power Up (Europe)
		 || (_gametid.rfind("TP4J", 0) == 0)	// Puyo Puyo!!: Puyopuyo 20th Anniversary (Japan)
		 || (_gametid.rfind("BYOJ", 0) == 0)	// Puyo Puyo 7 (Japan)
		 || (_gametid.rfind("BHXJ", 0) == 0)	// Quiz! Hexagon II (Japan)
		 || (_gametid.rfind("BQ2J", 0) == 0)	// Quiz Magic Academy DS: Futatsu no Jikuuseki (Japan)
		 || (_gametid.rfind("YRBK", 0) == 0)	// Ragnarok DS (Korea)
		 || (_gametid.rfind("TEDJ", 0) == 0)	// Red Stone DS: Akaki Ishi ni Michibikareshi Mono-tachi (Japan)
		 || (_gametid.rfind("B35J", 0) == 0)	// Rekishi Simulation Game: Sangokushi DS 3 (Japan)
		 || (_gametid.rfind("BUKJ", 0) == 0)	// Rekishi Taisen: Gettenka: Tenkaichi Battle Royale (Japan)
		 || (_gametid.rfind("YLZK", 0) == 0)	// Rhythm Sesang (Korea)
		 || (_gametid.rfind("BKMJ", 0) == 0)	// Rilakkuma Rhythm: Mattari Kibun de Dararan Ran (Japan)
		 || (_gametid.rfind("B6XJ", 0) == 0)	// Rockman EXE: Operate Shooting Star (Japan)
		 || (_gametid.rfind("V29J", 0) == 0)	// RPG Tkool DS (Japan)
		 || (_gametid.rfind("VEBJ", 0) == 0)	// RPG Tsukuru DS+: Create The New World (Japan)
		 || (_gametid.rfind("ARFK", 0) == 0)	// Rune Factory: Sinmokjjangiyagi (Korea)
		 || (_gametid.rfind("CSGJ", 0) == 0)	// SaGa 2: Hihou Densetsu: Goddess of Destiny (Japan)
		 || (_gametid.rfind("BZ3J", 0) == 0)	// SaGa 3: Jikuu no Hasha: Shadow or Light (Japan)
		 || (_gametid.rfind("CBEJ", 0) == 0)	// Saibanin Suiri Game: Yuuzai x Muzai (Japan)
		 || (_gametid.rfind("B59J", 0) == 0)	// Sakusaku Jinkou Kokyuu Care Training DS (Japan)
		 || (_gametid.rfind("BSWJ", 0) == 0)	// Saka Tsuku DS: World Challenge 2010 (Japan)
		 || (_gametid.rfind("B3GJ", 0) == 0)	// SD Gundam Sangoku Den: Brave Battle Warriors: Shin Militia Taisen (Japan)
		 || (_gametid.rfind("B7XJ", 0) == 0)	// Seitokai no Ichizon: DS Suru Seitokai (Japan)
		 || (_gametid.rfind("CQ2J", 0) == 0)	// Sengoku Spirits: Gunshi Den (Japan)
		 || (_gametid.rfind("CQ3J", 0) == 0)	// Sengoku Spirits: Moushou Den (Japan)
		 || (_gametid.rfind("YR4J", 0) == 0)	// Sengoku Spirits: Shukun Den (Japan)
		 || (_gametid.rfind("B5GJ", 0) == 0)	// Shin Sengoku Tenka Touitsu: Gunyuu-tachi no Souran (Japan)
		 || (_gametid.rfind("C36J", 0) == 0)	// Sloane to MacHale no Nazo no Story (Japan)
		 || (_gametid.rfind("B2QJ", 0) == 0)	// Sloane to MacHale no Nazo no Story 2 (Japan)
		 || (_gametid.rfind("A3YK", 0) == 0)	// Sonic Rush Adventure (Korea)
		 || (_gametid.rfind("TFLJ", 0) == 0)	// Sora no Otoshimono Forte: Dreamy Season (Japan)
		 || (_gametid.rfind("YW4K", 0) == 0)	// Spectral Force: Genesis (Korea)
		 || (_gametid.rfind("B22J", 0) == 0)	// Strike Witches 2: Iyasu, Naosu, Punipuni Suru (Japan)
		 || (_gametid.rfind("BYQJ", 0) == 0)	// Suisui Physical Assessment Training DS (Japan)
		 || (_gametid.rfind("TPQJ", 0) == 0)	// Suite PreCure: Melody Collection (Japan)
		 || (_gametid.rfind("CS7J", 0) == 0)	// Summon Night X: Tears Crown (Japan)
		 || (_gametid.rfind("C2YJ", 0) == 0)	// Supa Robo Gakuen (Japan) Nazotoki Adventure (Japan)
		 || (_gametid.rfind("BRWJ", 0) == 0)	// Super Robot Taisen L (Japan)
		 || (_gametid.rfind("BROJ", 0) == 0)	// Super Robot Taisen OG Saga: Masou Kishin: The Lord of Elemental (Japan)
		 || (_gametid.rfind("C5IJ", 0) == 0)	// Tago Akira no Atama no Taisou: Dai-1-shuu: Nazotoki Sekai Isshuu Ryokou (Japan)
		 || (_gametid.rfind("C52J", 0) == 0)	// Tago Akira no Atama no Taisou: Dai-2-shuu: Ginga Oudan (Japan)
		 || (_gametid.rfind("BQ3J", 0) == 0)	// Tago Akira no Atama no Taisou: Dai-3-shuu: Fushigi no Kuni no Nazotoki Otogibanashi (Japan)
		 || (_gametid.rfind("BQ4J", 0) == 0)	// Tago Akira no Atama no Taisou: Dai-4-shuu: Time Machine no Nazotoki Daibouken (Japan)
		 || (_gametid.rfind("B3DJ", 0) == 0)	// Taiko no Tatsujin DS: Dororon! Yookai Daikessen!! (Japan)
		 || (_gametid.rfind("B7KJ", 0) == 0)	// Tamagotch no Narikiri Challenge (Japan)
		 || (_gametid.rfind("BGVJ", 0) == 0)	// Tamagotch no Narikiri Channel (Japan)
		 || (_gametid.rfind("BG5J", 0) == 0)	// Tamagotch no Pichi Pichi Omisetchi (Japan)
		 || (_gametid.rfind("TGCJ", 0) == 0)	// Tamagotchi Collection (Japan)
		 || (_gametid.rfind("BQ9J", 0) == 0)	// Tekipaki Kyuukyuu Kyuuhen Training DS (Japan)
		 || (_gametid.rfind("B5KJ", 0) == 0)	// Tenkaichi: Sengoku Lovers DS (Japan)
		 || (_gametid.rfind("TENJ", 0) == 0)	// Tennis no Ouji-sama: Gyutto! Dokidoki Survival: Umi to Yama no Love Passion (Japan)
		 || (_gametid.rfind("BTGJ", 0) == 0)	// Tennis no Ouji-sama: Motto Gakuensai no Ouji-sama: More Sweet Edition (Japan)
		 || (_gametid.rfind("VIMJ", 0) == 0)	// The Idolm@ster: Dearly Stars (Japan)
		 || (_gametid.rfind("B6KP", 0) == 0)	// Tinker Bell + Tinker Bell and the Lost Treasure (Europe)
		 || (_gametid.rfind("TKGJ", 0) == 0)	// Tobidase! Kagaku-kun: Chikyuu Daitanken! Nazo no Chinkai Seibutsu ni Idome! (Japan)
		 || (_gametid.rfind("CT5K", 0) == 0)	// TOEIC DS: Haru 10-bun Yakjeomgeukbok +200 (Korea)
		 || (_gametid.rfind("AEYK", 0) == 0)	// TOEIC Test DS Training (Korea)
		 || (_gametid.rfind("BT5J", 0) == 0)	// TOEIC Test Super Coach@DS (Japan)
		 || (_gametid.rfind("TQ5J", 0) == 0)	// Tokumei Sentai Go Busters (Japan)
		 || (_gametid.rfind("CVAJ", 0) == 0)	// Tokyo Twilight Busters: Kindan no Ikenie Teito Jigokuhen (Japan)
		 || (_gametid.rfind("CZXK", 0) == 0)	// Touch Man to Man: Gichoyeongeo (Korea)
		 || (_gametid.rfind("BUQJ", 0) == 0)	// Treasure Report: Kikai Jikake no Isan (Japan)
		 || (_gametid.rfind("C2VJ", 0) == 0)	// Tsukibito (Japan)
		 || (_gametid.rfind("BH6J", 0) == 0)	// TV Anime Fairy Tail: Gekitou! Madoushi Kessen (Japan)
		 || (_gametid.rfind("CUHJ", 0) == 0)	// Umihara Kawase Shun: Second Edition Kanzen Ban (Japan)
		 || (_gametid.rfind("TBCJ", 0) == 0)	// Usavich: Game no Jikan (Japan)
		 || (_gametid.rfind("BPOJ", 0) == 0)	// Utacchi (Japan)
		 || (_gametid.rfind("BXPJ", 0) == 0)	// Winnie the Pooh: Kuma no Puu-san: 100 Acre no Mori no Cooking Book (Japan)
		 || (_gametid.rfind("BWYJ", 0) == 0)	// Wizardry: Boukyaku no Isan (Japan)
		 || (_gametid.rfind("BWZJ", 0) == 0)	// Wizardry: Inochi no Kusabi (Japan)
		 || (_gametid.rfind("BWWJ", 0) == 0)	// WiZmans World (Japan)
		 || (_gametid.rfind("BYNJ", 0) == 0)	// Yamakawa Shuppansha Kanshuu: Shousetsu Nihonshi B: Shin Sougou Training Plus (Japan)
		 || (_gametid.rfind("BYSJ", 0) == 0)	// Yamakawa Shuppansha Kanshuu: Shousetsu Sekaishi B: Shin Sougou Training Plus (Japan)
		 || (_gametid.rfind("B5DJ", 0) == 0)	// Yamanote-sen Meimei 100 Shuunen Kinen: Densha de Go!: Tokubetsu Hen: Fukkatsu! Shouwa no Yamanote-sen (Japan)
		 || (_gametid.rfind("BYMJ", 0) == 0)	// Yumeiro Patissiere: My Sweets Cooking (Japan)
		 || (_gametid.rfind("BZQJ", 0) == 0)	// Zac to Ombra: Maboroshi no Yuuenchi (Japan)
		 || (_gametid.rfind("BZBJ", 0) == 0))	// Zombie Daisuki (Japan) 
		 {
			 return APFixType::EMissingIPS;
		 }
		 static const char ap_list[][4] = {
				"YBN",	// 100 Classic Books
				"VAL",	// Alice in Wonderland
				"VAA",	// Art Academy
				"C7U",	// Battle of Giants: Dragons
				"BIG",	// Battle of Giants: Mutant Insects
				"BBU",	// Beyblade: Metal Fusion
				"BRZ",	// Beyblade: Metal Masters
				"YBU",	// Blue Dragon: Awakened Shadow
				"VKH",	// Brainstorm Series: Treasure Chase
				"BDU",	// C.O.P.: The Recruit
				"BDY",	// Call of Duty: Black Ops
				"TCM",	// Camping Mama: Outdoor Adventures
				"VCM",	// Camp Rock: The Final Jam
				"BQN",	// Captain America: Super Soldier
				"B2B",	// Captain Tsubasa
				"VCA",	// Cars 2
				"VMY",	// Chronicles of Mystery: The Secret Tree of Life
				"YQU",	// Chrono Trigger
				"CY9",	// Club Penguin: EPF: Herbert's Revenge
				"BQ8",	// Crafting Mama
				"VAO",	// Crime Lab: Body of Evidence
				"BD2",	// Deca Sports DS
				"BDE",	// Dementium II
				"BDB",	// Dragon Ball: Origins 2
				"YV5",	// Dragon Quest V: Hand of the Heavenly Bride
				"YVI",	// Dragon Quest VI: Realms of Revelation
				"YDQ",	// Dragon Quest IX: Sentinels of the Starry Skies
				"CJR",	// Dragon Quest Monsters: Joker 2
				"BEL",	// Element Hunters
				"BJ3",	// Etrian Odyssey III: The Drowned City
				"CFI",	// Final Fantasy Crystal Chronicles: Echoes of Time
				"BFX",	// Final Fantasy: The 4 Heroes of Light
				"VDE",	// Fossil Fighters Champions
				"BJC",	// GoldenEye 007
				"BO5",	// Golden Sun: Dark Dawn
				"YGX",	// Grand Theft Auto: Chinatown Wars
				"BGT",	// Ghost Trick: Phantom Detective
				"B7H",	// Harry Potter and the Deathly Hallows: Part 1
				"BU8",	// Harry Potter and the Deathly Hallows: Part 2
				"BKU",	// Harvest Moon DS: The Tale of Two Towns
				"YEE",	// Inazuma Eleven
				"BEE",	// Inazuma Eleven 2: Blizzard
				"BEB",	// Inazuma Eleven 2: Firestorm
				"B86",	// Jewels of the Ages
				"YKG",	// Kindgom Hearts: 358/2 Days
				"BK9",	// Kindgom Hearts: Re-coded
				"VKG",	// Korg DS-10+ Synthesizer
				"BQP",	// KuruKuru Princess: Tokimeki Figure
				"YLU", 	// Last Window: The Secret of Cape West
				"BSD",	// Lufia: Curse of the Sinistrals
				"YMP",	// MapleStory DS
				"CLJ",	// Mario & Luigi: Bowser's Inside Story
				"COL",	// Mario & Sonic at the Olympic Winter Games
				"V2G",	// Mario vs. Donkey Kong: Mini-Land Mayhem!
				"B6Z",	// Mega Man Zero Collection
				"BVN",	// Michael Jackson: The Experience
				"CHN",	// Might & Magic: Clash of Heroes
				"BNQ",	// Murder in Venice
				"BFL",	// MySims: Sky Heroes
				"CNS",	// Naruto Shippuden: Naruto vs Sasuke
				"BSK",	// Nine Hours: Nine Persons: Nine Doors
				"BOJ",	// One Piece: Gigant Battle!
				"BOO",	// Ookami Den
				"VFZ",	// Petz: Fantasy
				"BNR",	// Petz: Nursery
				"B3U",	// Petz: Nursery 2
				"C24",	// Phantasy Star 0
				"BZF",	// Phineas and Ferb: Across the 2nd Dimension
				"VPF",	// Phineas and Ferb: Ride Again
				"IPK",	// Pokemon HeartGold Version
				"IPG",	// Pokemon SoulSilver Version
				"IRA",	// Pokemon Black Version
				"IRB",	// Pokemon White Version
				"IRE",	// Pokemon Black Version 2
				"IRD",	// Pokemon White Version 2
				"VPY",	// Pokemon Conquest
				"B3R",	// Pokemon Ranger: Guardian Signs
				"VPP",	// Prince of Persia: The Forgotten Sands
				"BLF",	// Professor Layton and the Last Specter
				"C3J",	// Professor Layton and the Unwound Future
				"BKQ",	// Pucca: Power Up
				"VRG",	// Rabbids Go Home: A Comedy Adventure
				"BRJ",	// Radiant Hostoria
				"B3X",	// River City: Soccer Hooligans
				"BRM",	// Rooms: The Main Building
				"TDV",	// Shin Megami Tensei: Devil Survivor 2
				"BMT",	// Shin Megami Tensei: Strange Journey
				"VCD",	// Solatorobo: Red the Hunter
				"BXS",	// Sonic Colors
				"VSN",	// Sonny with a Chance
				"B2U",	// Sports Collection
				"CLW",	// Star Wars: The Clone Wars: Jedi Alliance
				"AZL",	// Style Savvy
				"BH2",	// Super Scribblenauts
				"B6T",	// Tangled
				"B4T",	// Tetris Party Deluxe
				"BKI",	// The Legend of Zelda: Spirit Tracks
				"VS3",	// The Sims 3
				"BZU",	// The Smurfs
				"TR2",	// The Smurfs 2
				"BS8",	// The Sorcerer's Apprentice
				"BTU",	// Tinker Bell and the Great Fairy Rescue
				"CCU",	// Tomodachi Collection
				"VT3",	// Toy Story 3
				"VTE",	// TRON: Evolution
				"B3V",	// Vampire Moon: The Mystery of the Hidden Sun
				"BW4",	// Wizards of Waverly Place: Spellbound
				"BYX",	// Yu-Gi-Oh! 5D's: World Championship 2010: Reverse of Arcadia
				"BYY",	// Yu-Gi-Oh! 5D's: World Championship 2011: Over the Nexus
			};
			// TODO: If the list gets large enough, switch to bsearch().
			for (unsigned int i = 0; i < sizeof(ap_list)/sizeof(ap_list[0]); i++) {
				if (strncmp(_gametid.c_str(), ap_list[i], 3) == 0) {
					// Found a match.
					return APFixType::EMissingIPS;
				}
			}
			static const char ap_list2[][4] = {
				"VID",	// Imagine: Resort Owner
				"TAD",	// Kirby: Mass Attack
			};
			// TODO: If the list gets large enough, switch to bsearch().
			for (unsigned int i = 0; i < sizeof(ap_list2)/sizeof(ap_list2[0]); i++) {
				if (strncmp(_gametid.c_str(), ap_list[i], 3) == 0) {
					// Found a match.
					return APFixType::ERGFPatch;
				}
			}
	return APFixType::ENone;
}