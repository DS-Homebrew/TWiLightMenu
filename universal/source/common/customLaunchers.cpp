/*
    common/customLaunchers.cpp

    See common/customLaunchers.h for the file format.
*/

#include "common/customLaunchers.h"

#include "common/bootstrappaths.h"
#include "common/inifile.h"
#include "common/logging.h"
#include "common/stringtool.h"
#include "common/systemdetails.h"

#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

static std::vector<CustomLauncher> sLaunchers;
static bool sLoaded = false;
static int sAddonConfigCount[4] = {0};

// The Nintendo DS binaries, which are always launched by the menus' own code
static const char *const kNdsFamilyExtensions[] = {
	".nds", ".dsi", ".ids", ".srl", ".app", ".argv"
};

// ICON names, indexed by bnrRomType number minus one
static const char *const kIconNames[] = {
	"gba", "gb", "gbc", "nes", "sms", "gg", "md", "snes", "plg", "a26", "pce", "int",
	"col", "m5", "sg", "ws", "ngp", "cpc", "vid", "img", "msx", "mini", "hb", "unk"
};

// The LAUNCHER_PATH_<NAME> keys each handler reads
static const char *const kFastVideoVariants[] = {"DLDI32"};
static const char *const kGbaVariants[] = {
	"NATIVE", "GBAR3", "DSI", "3DS", "DSI_NODSP", "3DS_NODSP",
	"DS_ARM7", "DS_ARM9", "DS_ARM7_ROM3M", "DS_ARM9_ROM3M"
};
static const char *const kS8dsColecoVariants[] = {"COLECODS"};
static const char *const kGenesisVariants[] = {"MACRO", "PICO"};
static const char *const kSnesVariants[] = {"TWL", "LEGACY", "LEGACY_TWLM"};

static bool endsWithNoCase(std::string_view str, const char *suffix)
{
	const size_t suffixLen = strlen(suffix);
	if (str.size() < suffixLen)
		return false;
	return strncasecmp(str.data() + str.size() - suffixLen, suffix, suffixLen) == 0;
}

static bool startsWithNoCase(const std::string &str, const char *prefix)
{
	const size_t prefixLen = strlen(prefix);
	if (str.size() < prefixLen)
		return false;
	return strncasecmp(str.c_str(), prefix, prefixLen) == 0;
}

static std::string toLower(std::string str)
{
	for (char &c : str)
		c = tolower((unsigned char)c);
	return str;
}

static std::string trim(const std::string &str)
{
	const size_t first = str.find_first_not_of(" \t");
	if (first == str.npos)
		return "";
	const size_t last = str.find_last_not_of(" \t");
	return str.substr(first, last - first + 1);
}

static std::vector<std::string> splitList(const std::string &str, char delimiter)
{
	std::vector<std::string> items;
	size_t start = 0;
	while (start <= str.size()) {
		size_t end = str.find(delimiter, start);
		if (end == str.npos)
			end = str.size();
		const std::string item = trim(str.substr(start, end - start));
		if (!item.empty())
			items.push_back(item);
		start = end + 1;
	}
	return items;
}

static bool isListedExtension(const std::string &ext, const char *const *list, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		if (strcasecmp(ext.c_str(), list[i]) == 0)
			return true;
	}
	return false;
}

// Directory listings, read once per directory while the configs load, so checking
// ~70 launcher paths on two devices costs a handful of directory reads instead of
// ~140 access() calls on slow flashcard FAT.
struct DirListing {
	std::string dir;			// lowercased, e.g. "fat:/_nds/twilightmenu/emulators"
	std::vector<std::string> names;		// lowercased
};
static std::vector<DirListing> *sDirCache = NULL;

static bool fileOnDevice(const char *dev, const std::string &rel)
{
	const std::string full = dev + rel;
	const size_t slash = full.find_last_of('/');
	if (slash == full.npos)
		return false;
	const std::string dirPath = full.substr(0, slash);	// opened as written; FAT ignores case
	const std::string dir = toLower(dirPath);
	const std::string name = toLower(full.substr(slash + 1));

	DirListing *listing = NULL;
	for (DirListing &cached : *sDirCache) {
		if (cached.dir == dir) {
			listing = &cached;
			break;
		}
	}
	if (!listing) {
		sDirCache->push_back(DirListing());
		listing = &sDirCache->back();
		listing->dir = dir;
		DIR *d = opendir((dirPath.find('/') == dirPath.npos ? dirPath + "/" : dirPath).c_str());
		if (d) {
			dirent *ent;
			while ((ent = readdir(d)) != NULL)
				listing->names.push_back(toLower(ent->d_name));
			closedir(d);
		}
	}

	for (const std::string &entry : listing->names) {
		if (entry == name)
			return true;
	}
	return false;
}

static LauncherBinary parseBinary(const std::string &value)
{
	LauncherBinary binary;
	std::string path = trim(value);
	if (path.empty())
		return binary;

	if (startsWithNoCase(path, "sd:")) {
		binary.device = 1;
		path.erase(0, 3);
	} else if (startsWithNoCase(path, "fat:")) {
		binary.device = 2;
		path.erase(0, 4);
	}
	if (path.empty()) {
		binary.device = 0;
		return binary;
	}
	binary.rel = (path[0] == '/') ? path : ("/" + path);

	binary.onSd = (binary.device != 2) && fileOnDevice("sd:", binary.rel);
	binary.onFat = (binary.device != 1) && fileOnDevice("fat:", binary.rel);
	return binary;
}

std::string LauncherBinary::on(const char *dev) const
{
	if (device == 1)
		return "sd:" + rel;
	if (device == 2)
		return "fat:" + rel;
	return dev + rel;
}

const LauncherBinary *CustomLauncher::variant(const char *name) const
{
	for (const LauncherVariant &v : variants) {
		if (v.name == name)
			return v.binary.empty() ? NULL : &v.binary;
	}
	return NULL;
}

static void readVariants(CIniFile &ini, CustomLauncher &launcher, const char *const *names, size_t count)
{
	for (size_t i = 0; i < count; i++) {
		LauncherVariant v;
		v.name = names[i];
		v.binary = parseBinary(ini.GetString("LAUNCHER", std::string("LAUNCHER_PATH_") + names[i], ""));
		if (!v.binary.empty())
			launcher.variants.push_back(v);
	}
}

static bool parseHandler(const std::string &value, LauncherHandler &handler)
{
	const std::string name = toLower(trim(value));
	if (name.empty() || name == "direct")
		handler = LauncherHandler::Direct;
	else if (name == "fastvideo")
		handler = LauncherHandler::FastVideo;
	else if (name == "gba")
		handler = LauncherHandler::Gba;
	else if (name == "s8ds-colecods")
		handler = LauncherHandler::S8dsColecoDS;
	else if (name == "genesis")
		handler = LauncherHandler::Genesis;
	else if (name == "snes")
		handler = LauncherHandler::Snes;
	else if (name == "pce")
		handler = LauncherHandler::Pce;
	else if (name == "dstwo-plugin")
		handler = LauncherHandler::DstwoPlugin;
	else
		return false;
	return true;
}

static bool parseListIf(const std::string &value, unsigned &listIf)
{
	listIf = 0;
	for (const std::string &token : splitList(toLower(value), ',')) {
		if (token == "april-fools")
			listIf |= LAUNCHER_LIST_IF_APRIL_FOOLS;
		else if (token == "dsi-features")
			listIf |= LAUNCHER_LIST_IF_DSI_FEATURES;
		else if (token == "dsi-console")
			listIf |= LAUNCHER_LIST_IF_DSI_CONSOLE;
		else if (token == "dstwo")
			listIf |= LAUNCHER_LIST_IF_DSTWO;
		else if (token == "sd-or-picodrive")
			listIf |= LAUNCHER_LIST_IF_SD_OR_PICODRIVE;
		else
			return false;
	}
	return true;
}

static int parseIcon(const std::string &value)
{
	const std::string name = toLower(trim(value));
	for (size_t i = 0; i < sizeof(kIconNames) / sizeof(kIconNames[0]); i++) {
		if (name == kIconNames[i])
			return (int)i + 1;
	}
	return LAUNCHER_ROM_TYPE_UNK;
}

static void loadOneConfig(const std::string &extrasDir, const std::string &fileName)
{
	// "config.ext.ini" -> ".ext"
	std::string ext = fileName.substr(strlen("config."),
					  fileName.size() - strlen("config.") - strlen(".ini"));
	if (ext.empty())
		return;
	ext = "." + toLower(ext);

	if (isListedExtension(ext, kNdsFamilyExtensions, sizeof(kNdsFamilyExtensions) / sizeof(kNdsFamilyExtensions[0])))
		return;

	for (const CustomLauncher &existing : sLaunchers) {
		if (existing.extension == ext)
			return; // first config wins
	}

	CIniFile ini(extrasDir + "/" + fileName);

	CustomLauncher launcher;
	launcher.extension = ext;

	const std::string addon = toLower(trim(ini.GetString("LAUNCHER", "ADDON", "")));
	if (addon == "base")
		launcher.addon = LauncherAddon::Base;
	else if (addon == "virtual-console")
		launcher.addon = LauncherAddon::VirtualConsole;
	else if (addon == "multimedia")
		launcher.addon = LauncherAddon::Multimedia;
	sAddonConfigCount[(int)launcher.addon]++;	// counted even if its launcher is missing

	if (!parseHandler(ini.GetString("LAUNCHER", "HANDLER", "direct"), launcher.handler)) {
		logPrint("%s: unknown HANDLER, ignored\n", fileName.c_str());
		return;
	}
	if (!parseListIf(ini.GetString("LAUNCHER", "LIST_IF", ""), launcher.listIf)) {
		logPrint("%s: unknown LIST_IF condition, ignored\n", fileName.c_str());
		return;
	}

	const std::string resolve = toLower(trim(ini.GetString("LAUNCHER", "RESOLVE", "sd-first")));
	if (resolve == "sd-first") {
		launcher.resolve = LauncherResolve::SdFirst;
	} else if (resolve == "boot-device") {
		launcher.resolve = LauncherResolve::BootDevice;
	} else {
		logPrint("%s: unknown RESOLVE, ignored\n", fileName.c_str());
		return;
	}

	launcher.binary = parseBinary(ini.GetString("LAUNCHER", "LAUNCHER_PATH", ""));
	switch (launcher.handler) {
		case LauncherHandler::FastVideo:
			readVariants(ini, launcher, kFastVideoVariants, sizeof(kFastVideoVariants) / sizeof(kFastVideoVariants[0]));
			break;
		case LauncherHandler::Gba:
			readVariants(ini, launcher, kGbaVariants, sizeof(kGbaVariants) / sizeof(kGbaVariants[0]));
			break;
		case LauncherHandler::S8dsColecoDS: {
			readVariants(ini, launcher, kS8dsColecoVariants, sizeof(kS8dsColecoVariants) / sizeof(kS8dsColecoVariants[0]));
			const std::string setting = toLower(trim(ini.GetString("LAUNCHER", "SETTING", "")));
			if (setting == "col") {
				launcher.setting = 0;
			} else if (setting == "sg") {
				launcher.setting = 1;
			} else {
				logPrint("%s: SETTING must be col or sg, ignored\n", fileName.c_str());
				return;
			}
			break;
		}
		case LauncherHandler::Genesis:
			readVariants(ini, launcher, kGenesisVariants, sizeof(kGenesisVariants) / sizeof(kGenesisVariants[0]));
			break;
		case LauncherHandler::Snes:
			readVariants(ini, launcher, kSnesVariants, sizeof(kSnesVariants) / sizeof(kSnesVariants[0]));
			break;
		default:
			break;
	}

	bool anyBinary = launcher.binary.exists();
	for (const LauncherVariant &v : launcher.variants)
		anyBinary = anyBinary || v.binary.exists();
	if (!anyBinary)
		return; // nothing to launch on the card, so never offer the extension at all

	const LauncherBinary banner = parseBinary(ini.GetString("LAUNCHER", "BANNER_PATH", ""));
	if (banner.exists()) {
		const bool preferSd = sys().isRunFromSD();
		launcher.bannerPath = ((preferSd && banner.onSd) || !banner.onFat) ? banner.on("sd:") : banner.on("fat:");
	}
	launcher.bannerIsPng = endsWithNoCase(launcher.bannerPath, ".png");
	launcher.romType = parseIcon(ini.GetString("LAUNCHER", "ICON", "unk"));

	launcher.arg = ini.GetString("LAUNCHER", "ARG", "%PATH%");
	launcher.extraArgs = ini.GetString("LAUNCHER", "EXTRA_ARGS", "");
	launcher.boostCpu = ini.GetInt("LAUNCHER", "BOOST_CPU", 1);
	launcher.boostVram = ini.GetInt("LAUNCHER", "BOOST_VRAM", -1);
	launcher.dsMode = ini.GetInt("LAUNCHER", "DS_MODE", 0);
	launcher.ntrSdRelaunch = (ini.GetInt("LAUNCHER", "NTR_SD_RELAUNCH", 1) != 0);
	for (std::string dir : splitList(ini.GetString("LAUNCHER", "DATA_DIRS", ""), ',')) {
		while (!dir.empty() && dir[0] == '/')
			dir.erase(0, 1);
		while (!dir.empty() && dir[dir.size() - 1] == '/')
			dir.erase(dir.size() - 1);
		if (!dir.empty())
			launcher.dataDirs.push_back(dir);
	}

	sLaunchers.push_back(launcher);
}

void loadCustomLaunchers(void)
{
	if (sLoaded)
		return;
	sLoaded = true;

	const std::string extrasDir =
		std::string(sys().isRunFromSD() ? "sd:" : "fat:") + TWLMENU_EXTRAS_DIR;

	DIR *dir = opendir(extrasDir.c_str());
	if (!dir)
		return;

	std::vector<DirListing> dirCache;
	sDirCache = &dirCache;

	dirent *ent;
	while ((ent = readdir(dir)) != NULL) {
		const std::string name = ent->d_name;
		// "config.example.ini.txt" and the other extras/*.ini files are skipped here
		if (!startsWithNoCase(name, "config.") || !endsWithNoCase(name, ".ini"))
			continue;
		if (name.size() <= strlen("config.") + strlen(".ini"))
			continue;
		loadOneConfig(extrasDir, name);
	}

	closedir(dir);
	sDirCache = NULL;
}

const std::vector<CustomLauncher> &customLaunchers(void)
{
	return sLaunchers;
}

const CustomLauncher *findCustomLauncher(std::string_view filename)
{
	// Longest match wins, so a ".lz77.gen" config beats a ".gen" one regardless of the
	// order the configs happened to be read in.
	const CustomLauncher *best = NULL;

	for (const CustomLauncher &launcher : sLaunchers) {
		const std::string &ext = launcher.extension;
		if (filename.size() <= ext.size())
			continue;
		if (best && best->extension.size() >= ext.size())
			continue;
		if (strncasecmp(filename.data() + filename.size() - ext.size(), ext.c_str(),
				ext.size()) == 0)
			best = &launcher;
	}

	return best;
}

bool isNdsFamilyFile(std::string_view filename)
{
	for (const char *ext : kNdsFamilyExtensions) {
		if (filename.size() > strlen(ext) && endsWithNoCase(filename, ext))
			return true;
	}
	return false;
}

int launcherRomType(std::string_view filename)
{
	if (isNdsFamilyFile(filename))
		return LAUNCHER_ROM_TYPE_NDS;
	const CustomLauncher *launcher = findCustomLauncher(filename);
	return launcher ? launcher->romType : LAUNCHER_ROM_TYPE_UNK;
}

int launcherConfigCount(LauncherAddon addon)
{
	return sAddonConfigCount[(int)addon];
}

std::string buildLauncherArg(const CustomLauncher &launcher, const std::string &romPath,
			     const std::string &romPathFat, const std::string &filename)
{
	if (launcher.arg.empty())
		return "";

	std::string nameNoExt = filename;
	const size_t dot = nameNoExt.find_last_of('.');
	if (dot != nameNoExt.npos)
		nameNoExt.erase(dot);

	std::string dirPart = romPath;
	const size_t slash = dirPart.find_last_of('/');
	if (slash != dirPart.npos)
		dirPart.erase(slash);

	std::string arg = launcher.arg;
	arg = replaceAll(arg, "%PATH_FAT%", romPathFat.empty() ? romPath : romPathFat);
	arg = replaceAll(arg, "%PATH%", romPath);
	arg = replaceAll(arg, "%NAME_NOEXT%", nameNoExt);
	arg = replaceAll(arg, "%NAME%", filename);
	arg = replaceAll(arg, "%DIR%", dirPart);

	return arg;
}

std::vector<std::string> splitLauncherExtraArgs(const CustomLauncher &launcher)
{
	std::vector<std::string> args;

	size_t pos = 0;
	while (pos < launcher.extraArgs.size()) {
		const size_t start = launcher.extraArgs.find_first_not_of(" \t", pos);
		if (start == launcher.extraArgs.npos)
			break;
		size_t end = launcher.extraArgs.find_first_of(" \t", start);
		if (end == launcher.extraArgs.npos)
			end = launcher.extraArgs.size();
		args.push_back(launcher.extraArgs.substr(start, end - start));
		pos = end;
	}

	return args;
}
