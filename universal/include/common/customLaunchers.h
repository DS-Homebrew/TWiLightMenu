/*
    common/customLaunchers.h

    Per-extension file-type launchers.

    "config.<ext>.ini" files in "/_nds/TWiLightMenu/extras" tell TWiLight Menu++ how to
    list, show and launch a file extension:

        [LAUNCHER]
        HANDLER=direct
        LAUNCHER_PATH=/_nds/TWiLightMenu/emulators/nesds.nds
        ICON=nes
        BANNER_PATH=
        ARG=%PATH%
        EXTRA_ARGS=
        BOOST_CPU=1
        BOOST_VRAM=-1
        DS_MODE=0
        DATA_DIRS=
        RESOLVE=sd-first
        NTR_SD_RELAUNCH=1
        LIST_IF=
        ADDON=virtual-console

    HANDLER=direct boots LAUNCHER_PATH as described by the keys above. The other handlers
    (fastvideo, gba, s8ds-colecods, genesis, snes, pce, dstwo-plugin) carry launch recipes
    too involved for plain keys, and pick between LAUNCHER_PATH and LAUNCHER_PATH_<VARIANT>
    binaries. s8ds-colecods also takes SETTING=col or SETTING=sg, the Settings choice it
    follows. See universal/source/launch/launchPlanner.cpp for what each handler does.

    ICON picks the theme's icon: gba gb gbc nes sms gg md snes plg a26 pce int col m5 sg ws
    ngp cpc vid img msx mini hb unk. BANNER_PATH (a .bin NDS banner or a .png) replaces it.
    ADDON (base, virtual-console, multimedia) names the package the config ships in, so
    title can tell when a package predates its configs.

    Every file type other than Nintendo DS binaries is listed and launched through these
    configs, including the stock emulators: the base package and the Virtual Console and
    Multimedia add-ons ship one per extension. An extension without a config, or whose
    launcher isn't on the card, isn't listed.

    Nintendo DS binaries (.nds .dsi .ids .srl .app, and .argv) are never handled here.
*/

#ifndef _CUSTOMLAUNCHERS_H_
#define _CUSTOMLAUNCHERS_H_

#include <string>
#include <string_view>
#include <vector>

enum class LauncherHandler {
	Direct,
	FastVideo,
	Gba,
	S8dsColecoDS,
	Genesis,
	Snes,
	Pce,
	DstwoPlugin
};

enum class LauncherResolve {
	SdFirst,	// sd: in DSi mode when the file is there, otherwise fat: (the stock emulator rule)
	BootDevice	// the device TWiLight Menu++ was booted from
};

enum class LauncherAddon {
	None,
	Base,
	VirtualConsole,
	Multimedia
};

// LIST_IF conditions; all that are set must hold for the extension to be listed
enum : unsigned {
	LAUNCHER_LIST_IF_APRIL_FOOLS     = 1 << 0,	// april-fools: the date is 04/01
	LAUNCHER_LIST_IF_DSI_FEATURES    = 1 << 1,	// dsi-features
	LAUNCHER_LIST_IF_DSI_CONSOLE     = 1 << 2,	// dsi-console: not a 3DS
	LAUNCHER_LIST_IF_DSTWO           = 1 << 3,	// dstwo: a DSTWO is the Slot-1 flashcard
	LAUNCHER_LIST_IF_SD_OR_PICODRIVE = 1 << 4	// sd-or-picodrive: ROM on SD, or PicoDrive TWL selected
};

// File type numbers, as stored in bnrRomType by every menu (quickmenu's eROMType matches)
enum : int {
	LAUNCHER_ROM_TYPE_NDS = 0,
	LAUNCHER_ROM_TYPE_UNK = 24
};

struct LauncherBinary {
	std::string rel;	// path without a device, starting with '/'; empty if not configured
	int device = 0;		// 0 = none given, 1 = "sd:" given, 2 = "fat:" given
	bool onSd = false;	// found on sd: when the configs were loaded
	bool onFat = false;	// found on fat: when the configs were loaded

	bool empty() const { return rel.empty(); }
	bool exists() const { return onSd || onFat; }
	// The path on "sd:" or "fat:". A path written with an explicit device stays on it.
	std::string on(const char *dev) const;
};

struct LauncherVariant {
	std::string name;	// "DLDI32" for LAUNCHER_PATH_DLDI32
	LauncherBinary binary;
};

struct CustomLauncher {
	std::string extension;	// ".ext", lowercased, with the leading dot
	LauncherHandler handler = LauncherHandler::Direct;
	LauncherBinary binary;			// LAUNCHER_PATH
	std::vector<LauncherVariant> variants;	// the LAUNCHER_PATH_<NAME> keys the handler uses
	LauncherResolve resolve = LauncherResolve::SdFirst;
	std::string arg;	// argv template, "" means pass no argument
	std::string extraArgs;	// space separated literal args, placed before arg
	int boostCpu = 1;	// 0/1
	int boostVram = -1;	// 0/1, or -1: boost when the launcher runs from fat: or in DS mode
	int dsMode = 0;		// 0/1, or -1: switch to DS mode when not already in DSi mode
	std::vector<std::string> dataDirs;	// created on the ROM's device before launching, e.g. "data/s8ds"
	bool ntrSdRelaunch = true;	// in DS mode, relaunch through TWiLight Menu++ to reach an SD ROM
	unsigned listIf = 0;	// LAUNCHER_LIST_IF_* bits
	int romType = LAUNCHER_ROM_TYPE_UNK;	// ICON, as a bnrRomType number
	std::string bannerPath;	// resolved, or empty if none was given
	bool bannerIsPng = false;	// false = raw NDS banner (.bin), true = .png
	LauncherAddon addon = LauncherAddon::None;
	int setting = 0;	// s8ds-colecods: 0 = col, 1 = sg

	// A LAUNCHER_PATH_<name> binary, or NULL if it isn't configured
	const LauncherBinary *variant(const char *name) const;
};

// Scans the extras folder once. Safe to call more than once; later calls are no-ops.
void loadCustomLaunchers(void);

// The loaded configs. Their strings stay alive for the rest of the session, so
// std::string_view's into them (e.g. in extensionList) remain valid.
const std::vector<CustomLauncher> &customLaunchers(void);

// Matches filename's extension against the loaded configs, case-insensitively.
// The longest matching extension wins.
const CustomLauncher *findCustomLauncher(std::string_view filename);

// True for the Nintendo DS binaries that never go through a config
bool isNdsFamilyFile(std::string_view filename);

// The bnrRomType for a file: LAUNCHER_ROM_TYPE_NDS for DS binaries, the config's ICON
// otherwise, or LAUNCHER_ROM_TYPE_UNK if no config handles the file.
int launcherRomType(std::string_view filename);

// How many config files naming this ADDON were read, whether or not their launchers
// are present. Used to spot an add-on that predates these configs.
int launcherConfigCount(LauncherAddon addon);

// Expands the ARG template. romPath is the file as launched ("sd:/roms/x/a.ext"),
// romPathFat the same in "fat:/" form, filename the bare name with extension.
std::string buildLauncherArg(const CustomLauncher &launcher, const std::string &romPath,
			     const std::string &romPathFat, const std::string &filename);

// Splits EXTRA_ARGS on spaces. Returns an empty vector when there are none.
std::vector<std::string> splitLauncherExtraArgs(const CustomLauncher &launcher);

#endif // _CUSTOMLAUNCHERS_H_
