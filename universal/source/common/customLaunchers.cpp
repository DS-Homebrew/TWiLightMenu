/*
    common/customLaunchers.cpp

    See common/customLaunchers.h for the file format.
*/

#include "common/customLaunchers.h"

#include "common/bootstrappaths.h"
#include "common/inifile.h"
#include "common/stringtool.h"
#include "common/systemdetails.h"

#include <ctype.h>
#include <dirent.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

static std::vector<CustomLauncher> sLaunchers;
static bool sLoaded = false;

// Every extension TWiLight Menu++ handles itself. A config naming one of these is
// ignored, so a bad ini can never break stock behaviour. Keep in sync with the
// extensionList blocks in each theme's main.cpp.
static const char *kBuiltinExtensions[] = {
	".nds", ".dsi", ".ids", ".srl", ".app", ".argv",		// NDS
	".agb", ".gba", ".mb",						// GBA
	".3ds", ".cia", ".cxi", ".ntrb",				// April Fools
	".plg",								// DSTWO plugin
	".a26", ".a52", ".a78", ".xex", ".atr",				// Atari
	".msx", ".col", ".int", ".m5",					// MSX/Coleco/Intellivision/Sord
	".sg", ".sc", ".sms", ".gg", ".gen", ".md",			// Sega
	".gb", ".sgb", ".gbc", ".nes", ".fds",				// Nintendo
	".smc", ".sfc", ".pce", ".ws", ".wsc",
	".ngp", ".ngc", ".dsk", ".min",
	".avi", ".rvid", ".fv", ".gif", ".bmp", ".png"			// Multimedia
};

static bool endsWithNoCase(const std::string &str, const char *suffix)
{
	const size_t suffixLen = strlen(suffix);
	if (str.size() < suffixLen)
		return false;
	return strncasecmp(str.c_str() + str.size() - suffixLen, suffix, suffixLen) == 0;
}

static bool startsWithNoCase(const std::string &str, const char *prefix)
{
	const size_t prefixLen = strlen(prefix);
	if (str.size() < prefixLen)
		return false;
	return strncasecmp(str.c_str(), prefix, prefixLen) == 0;
}

static bool isBuiltinExtension(const std::string &ext)
{
	for (const char *builtin : kBuiltinExtensions) {
		if (strcasecmp(ext.c_str(), builtin) == 0)
			return true;
	}
	return false;
}

// Turns "/_nds/TWiLightMenu/apps/x.nds" into a path on whichever device actually
// holds the file, preferring the one TWiLight Menu++ is running from. A path that
// already names a device is used as-is. Returns false if the file is not there.
static bool resolvePath(const std::string &path, std::string &out)
{
	if (path.empty())
		return false;

	if (startsWithNoCase(path, "sd:/") || startsWithNoCase(path, "fat:/")) {
		if (access(path.c_str(), F_OK) == 0) {
			out = path;
			return true;
		}
		return false;
	}

	const std::string rel = (path[0] == '/') ? path : ("/" + path);
	const char *primary = sys().isRunFromSD() ? "sd:" : "fat:";
	const char *secondary = sys().isRunFromSD() ? "fat:" : "sd:";

	std::string candidate = primary + rel;
	if (access(candidate.c_str(), F_OK) == 0) {
		out = candidate;
		return true;
	}

	candidate = secondary + rel;
	if (access(candidate.c_str(), F_OK) == 0) {
		out = candidate;
		return true;
	}

	return false;
}

static void loadOneConfig(const std::string &extrasDir, const std::string &fileName)
{
	// "config.ext.ini" -> ".ext"
	std::string ext = fileName.substr(strlen("config."),
					  fileName.size() - strlen("config.") - strlen(".ini"));
	if (ext.empty())
		return;

	for (char &c : ext)
		c = tolower((unsigned char)c);
	ext = "." + ext;

	if (isBuiltinExtension(ext))
		return;

	for (const CustomLauncher &existing : sLaunchers) {
		if (existing.extension == ext)
			return; // first config wins
	}

	CIniFile ini(extrasDir + "/" + fileName);

	CustomLauncher launcher;
	launcher.extension = ext;

	if (!resolvePath(ini.GetString("LAUNCHER", "LAUNCHER_PATH", ""), launcher.launcherPath))
		return; // no launcher on the card, so never offer the extension at all

	const std::string bannerPath = ini.GetString("LAUNCHER", "BANNER_PATH", "");
	if (!bannerPath.empty() && !resolvePath(bannerPath, launcher.bannerPath))
		launcher.bannerPath.clear(); // missing banner is not fatal, just fall back
	launcher.bannerIsPng = endsWithNoCase(launcher.bannerPath, ".png");

	launcher.arg = ini.GetString("LAUNCHER", "ARG", "%PATH%");
	launcher.extraArgs = ini.GetString("LAUNCHER", "EXTRA_ARGS", "");
	launcher.boostCpu = ini.GetInt("LAUNCHER", "BOOST_CPU", 1);
	launcher.boostVram = ini.GetInt("LAUNCHER", "BOOST_VRAM", -1);
	launcher.dsMode = ini.GetInt("LAUNCHER", "DS_MODE", 0);

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
}

const std::vector<CustomLauncher> &customLaunchers(void)
{
	return sLaunchers;
}

const CustomLauncher *findCustomLauncher(std::string_view filename)
{
	// Longest match wins, so a ".tar.gz" config beats a ".gz" one regardless of the
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
