/*
    launch/launchPlanner.h

    Decides how to launch a file through its extras/config.<ext>.ini launcher: which
    binary to boot, with which arguments and flags, and what has to happen first.

    Planning is pure: everything it depends on is passed in a LaunchEnv, so it can be
    tested on a host. launch/launchExecutor.h captures the environment on a console
    and carries a plan out.

    Only compiled into the binaries that launch files: the three ROM-select themes,
    the Quick Menu and title.
*/

#ifndef _LAUNCHPLANNER_H_
#define _LAUNCHPLANNER_H_

#include <string>
#include <string_view>
#include <vector>

#include "common/customLaunchers.h"

// Values of TWLSettings::TLaunchType a plan can set (checked against the enum in launchExecutor.cpp)
enum : int {
	LAUNCH_TYPE_UNCHANGED = -1,
	LAUNCH_TYPE_SD_FLASHCARD = 1,	// ESDFlashcardLaunch: booted through nds-bootstrap-hb
	LAUNCH_TYPE_GBA_NATIVE = 11,	// EGBANativeLaunch: GBA ROM copied to a Slot-2 cart
	LAUNCH_TYPE_CUSTOM = 28		// ECustomLaunch: anything else a config launched
};

struct LaunchEnv {
	// Console and boot state
	bool isDSiMode = false;
	bool dsiFeatures = false;
	bool arm7SCFGLocked = false;
	bool dsiWramAccess = false;
	bool sdFound = false;
	bool flashcardFound = false;
	bool isRunFromSD = false;
	int consoleModel = 0;
	unsigned dldiDriverSize = 0;
	bool dldiIsDstwo = false;
	bool dldiIsCycloDsi = false;	// CycloDS iEvolution, which runs nds-bootstrap-hb with SCFG locked
	unsigned slot2Id = 0;		// 0 = no Slot-2 RAM, 0x5A45 = EZ-Flash NOR, 0x4353 = SuperCard
	bool aprilFools = false;

	// The ROM's device: true = flashcard (fat:), false = SD
	bool secondaryDevice = false;

	// Settings the handlers read
	int gbaBooter = 0;		// 1 = copy GBA ROMs to a Slot-2 cart when one is inserted
	int colEmulator = 0;
	int sgEmulator = 0;
	int mdEmulator = 0;
	bool newSnesEmuVer = false;
	bool smsGgInRam = false;
	bool gbar2DldiAccess = false;
	bool macroMode = false;

	// File access, so planning can run against a fake card
	unsigned (*fileSize)(const char *path) = nullptr;
	bool (*readGbaTid)(const char *romPath, char tid[4]) = nullptr;
};

struct LaunchRequest {
	std::string romFolder;			// without a trailing slash, e.g. "sd:/roms/nes"
	std::string filename;			// e.g. "Game.nes"
	std::vector<std::string> prefixArgs;	// arguments from a .argv file, placed first
	bool resume = false;			// true when title relaunches the last played file
};

struct LaunchPlan {
	bool ok = false;

	std::string romPath;		// romFolder + "/" + filename
	std::string romPathFat;		// romPath in "fat:/" form

	std::string ndsToBoot;		// the emulator or app
	std::string ndsToBootFat;	// ndsToBoot in "fat:/" form, for nds-bootstrap-hb
	std::string argv0;		// argv[0] when it isn't ndsToBoot (TGDS multiboot)
	std::vector<std::string> args;	// argv[1] onwards

	bool useNDSB = false;		// boot ndsToBoot through nds-bootstrap-hb
	bool tgdsMode = false;
	bool tscTgds = false;
	bool dsModeSwitch = false;
	bool boostCpu = true;
	bool boostVram = false;
	int romToRamDisk = -1;
	bool romIsCompressed = false;
	bool ntrSdRelaunch = true;	// in DS mode, relaunch through TWiLight Menu++ to reach an SD ROM

	int launchType = LAUNCH_TYPE_UNCHANGED;
	std::string homebrewArg;	// saved as HOMEBREW_ARG

	std::vector<std::string> mkdirs;	// created in order before launching

	// nds-bootstrap.ini, when useNDSB
	bool writeBootstrapIni = false;
	std::string bootstrapHomebrewArg;
	std::string bootstrapRamDrivePath;
	int bootstrapBoostCpu = 1;

	// DSTWO plugin: written to fat:/_dstwo/twlm.ini [boot_settings] file, once a plugin
	// on SD has been copied to dstwoCopyTo on the DSTWO
	std::string dstwoBootFile;
	std::string dstwoCopyTo;

	// GBA ROM copied to the Slot-2 cart, with its save in the cart's SRAM, before booting
	// ndsToBoot (gbapatcher). On resume the ROM is still in an EZ-Flash's NOR flash.
	bool gbaNative = false;
	bool gbaNativeKeepNor = false;
	std::string gbaSavePath;
};

// All LIST_IF conditions hold
bool launcherListIfHolds(const CustomLauncher &launcher, const LaunchEnv &env);

// The binaries needed to launch in this environment are on the card
bool launcherAvailable(const CustomLauncher &launcher, const LaunchEnv &env);

// Adds the extension of every config that should show up in the file browser
void appendLauncherExtensions(std::vector<std::string_view> &extensionList, const LaunchEnv &env);

// What a file browser checks before launching a file through its launcher
enum class LaunchPrecheck {
	None,
	GbaBios,	// GBARunner may need the GBA BIOS: the menu's checkForGbaBiosRequirement() decides
	MdRomTooBig,	// jEnesisDS can't play Genesis ROMs over 3 MB
	LockedScfg	// SNEmulDS, and NitroGrafx with the ROM in RAM, can't run with SCFG locked
};

// launcher may be NULL (no config), which needs no check. romPath may be relative to the current folder.
LaunchPrecheck launcherPrecheck(const CustomLauncher *launcher, const LaunchEnv &env, const std::string &romPath);

// Plans a launch. plan.ok is false if the config can't launch in this environment.
LaunchPlan planLaunch(const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request);

// Whether title should relaunch the last played file through its config, given the
// LAUNCH_TYPE saved for it. Also true for the per-emulator launch types saved by
// older versions. nds-bootstrap-hb launches from SD are left to title's own
// ESDFlashcardLaunch code, which boots from the nds-bootstrap.ini the menu wrote.
bool launchTypeUsesLauncherConfig(int launchType, std::string_view romPath, bool secondaryDevice);

#endif // _LAUNCHPLANNER_H_
