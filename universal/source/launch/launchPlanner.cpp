/*
    launch/launchPlanner.cpp

    See launch/launchPlanner.h. The handler recipes mirror what
    romsel_dsimenutheme/arm9/source/main.cpp did for each extension before the
    launchers moved onto configs.
*/

#include "launch/launchPlanner.h"

#include <string.h>
#include <strings.h>

static std::string toFat(const std::string &path)
{
	if (path.compare(0, 4, "sd:/") == 0)
		return "fat:/" + path.substr(4);
	return path;
}

// The binary on "sd:" in DSi mode if it is there, otherwise on "fat:". This is the
// rule every built-in emulator used. A path with an explicit device stays on it.
static std::string resolveSdFirst(const LauncherBinary &binary, const LaunchEnv &env, bool &onFat)
{
	if (binary.device == 1 || binary.device == 2) {
		onFat = (binary.device == 2);
		return binary.on("sd:");
	}
	onFat = !(env.isDSiMode && binary.onSd);
	return binary.on(onFat ? "fat:" : "sd:");
}

static const char *bootDevice(const LaunchEnv &env)
{
	return env.isRunFromSD ? "sd:" : "fat:";
}

static bool availableOnBootDevice(const LauncherBinary &binary, const LaunchEnv &env)
{
	if (binary.device == 1)
		return binary.onSd;
	if (binary.device == 2)
		return binary.onFat;
	return env.isRunFromSD ? binary.onSd : binary.onFat;
}

static bool boolSetting(int value, bool automatic)
{
	return (value == -1) ? automatic : (value != 0);
}

bool launcherListIfHolds(const CustomLauncher &launcher, const LaunchEnv &env)
{
	const unsigned listIf = launcher.listIf;
	if ((listIf & LAUNCHER_LIST_IF_APRIL_FOOLS) && !env.aprilFools)
		return false;
	if ((listIf & LAUNCHER_LIST_IF_DSI_FEATURES) && !env.dsiFeatures)
		return false;
	if ((listIf & LAUNCHER_LIST_IF_DSI_CONSOLE) && env.consoleModel >= 2)
		return false;
	if ((listIf & LAUNCHER_LIST_IF_DSTWO) && !env.dldiIsDstwo)
		return false;
	if ((listIf & LAUNCHER_LIST_IF_SD_OR_PICODRIVE) && env.secondaryDevice && env.mdEmulator != 2)
		return false;
	return true;
}

static bool variantExists(const CustomLauncher &launcher, const char *name)
{
	const LauncherBinary *binary = launcher.variant(name);
	return binary && binary->exists();
}

// s8ds-colecods: ColecoDS when the Settings choice for the file type (colEmulator for
// SETTING=col, sgEmulator for SETTING=sg) is ColecoDS (2), otherwise S8DS
static const LauncherBinary &s8dsColecoDSBinary(const CustomLauncher &launcher, const LaunchEnv &env)
{
	const int emulator = (launcher.setting == 1) ? env.sgEmulator : env.colEmulator;
	const LauncherBinary *colecoDS = launcher.variant("COLECODS");
	return (emulator == 2 && colecoDS) ? *colecoDS : launcher.binary;
}

// genesis: jEnesisDS, or its macro build (LAUNCHER_PATH_MACRO) with Settings' macro mode on
static const LauncherBinary &jEnesisBinary(const CustomLauncher &launcher, const LaunchEnv &env)
{
	const LauncherBinary *macro = launcher.variant("MACRO");
	return (env.macroMode && macro) ? *macro : launcher.binary;
}

// genesis: PicoDrive TWL (LAUNCHER_PATH_PICO) is forced by a locked SCFG in DSi mode, chosen
// in Settings, or picked by the Hybrid setting for ROMs over 3 MB
static bool genesisPicoForced(const LaunchEnv &env)
{
	return env.isDSiMode && env.sdFound && env.arm7SCFGLocked;
}

static bool genesisUsesPicoDrive(const LaunchEnv &env, const std::string &romPath)
{
	return genesisPicoForced(env) || env.mdEmulator == 2
		|| (env.mdEmulator == 3 && env.fileSize && env.fileSize(romPath.c_str()) > 0x300000);
}

// gba: Settings' native GBA mode with a Slot-2 RAM or NOR cart inserted
static bool gbaUsesSlot2(const LaunchEnv &env)
{
	return env.gbaBooter == 1 && env.slot2Id != 0;
}

// gba: GBARunner3 (LAUNCHER_PATH_GBAR3) whenever it is on the device TWiLight Menu++ booted from
static const LauncherBinary *gbaRunner3(const CustomLauncher &launcher, const LaunchEnv &env)
{
	const LauncherBinary *gbar3 = launcher.variant("GBAR3");
	return (gbar3 && availableOnBootDevice(*gbar3, env)) ? gbar3 : NULL;
}

// gba: the GBARunner2 build for this console. The DSi and 3DS builds boot from SD through
// nds-bootstrap-hb, which needs the no-DSP build without SCFG or DSi WRAM access; DS builds
// read the ROM through the flashcard's DLDI from ARM7 or ARM9, and Pokémon Emerald needs 3 MB of RAM.
static const char *gbaRunner2Name(const LaunchEnv &env, bool bootstrapHb, bool pokemonEmerald)
{
	if (env.dsiFeatures || bootstrapHb) {
		const bool noDsp = bootstrapHb && env.isDSiMode && env.arm7SCFGLocked && !env.dsiWramAccess;
		if (env.consoleModel > 0)
			return noDsp ? "3DS_NODSP" : "3DS";
		return noDsp ? "DSI_NODSP" : "DSI";
	}
	if (pokemonEmerald)
		return env.gbar2DldiAccess ? "DS_ARM7_ROM3M" : "DS_ARM9_ROM3M";
	return env.gbar2DldiAccess ? "DS_ARM7" : "DS_ARM9";
}

bool launcherAvailable(const CustomLauncher &launcher, const LaunchEnv &env)
{
	switch (launcher.handler) {
		case LauncherHandler::Direct:
			if (launcher.resolve == LauncherResolve::BootDevice)
				return availableOnBootDevice(launcher.binary, env);
			return launcher.binary.exists();
		case LauncherHandler::FastVideo:
			return launcher.binary.exists() || variantExists(launcher, "DLDI32");
		case LauncherHandler::S8dsColecoDS:
			return s8dsColecoDSBinary(launcher, env).exists();
		case LauncherHandler::DstwoPlugin:
		case LauncherHandler::Pce:
			return launcher.binary.exists();
		case LauncherHandler::Snes:
			if (env.newSnesEmuVer)
				return launcher.binary.exists() || variantExists(launcher, "TWL");
			return variantExists(launcher, "LEGACY") || (env.secondaryDevice && variantExists(launcher, "LEGACY_TWLM"));
		case LauncherHandler::Genesis:
			// The ROM size isn't known here, so Hybrid needs either emulator
			if (genesisPicoForced(env) || env.mdEmulator == 2)
				return variantExists(launcher, "PICO");
			if (env.mdEmulator == 3)
				return jEnesisBinary(launcher, env).exists() || variantExists(launcher, "PICO");
			return jEnesisBinary(launcher, env).exists();
		case LauncherHandler::Gba:
			if (gbaUsesSlot2(env))
				return variantExists(launcher, "NATIVE");
			if (gbaRunner3(launcher, env))
				return true;
			// Pokémon Emerald's build isn't checked, as the ROM isn't known here
			return variantExists(launcher, gbaRunner2Name(env, !env.secondaryDevice, false));
		default:
			return false;	// handler not supported yet
	}
}

void appendLauncherExtensions(std::vector<std::string_view> &extensionList, const LaunchEnv &env)
{
	for (const CustomLauncher &launcher : customLaunchers()) {
		if (launcherListIfHolds(launcher, env) && launcherAvailable(launcher, env))
			extensionList.emplace_back(launcher.extension);
	}
}

// ARG and EXTRA_ARGS after any .argv arguments. Returns the expanded ARG.
static std::string addConfigArgs(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchRequest &request)
{
	for (const std::string &arg : request.prefixArgs)
		plan.args.push_back(arg);
	for (const std::string &arg : splitLauncherExtraArgs(launcher))
		plan.args.push_back(arg);
	const std::string arg = buildLauncherArg(launcher, plan.romPath, plan.romPathFat, request.filename);
	if (!arg.empty())
		plan.args.push_back(arg);
	return arg;
}

static void addDataDirs(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env)
{
	const std::string root = env.secondaryDevice ? "fat:" : "sd:";
	for (const std::string &dataDir : launcher.dataDirs) {
		size_t slash = 0;
		do {
			slash = dataDir.find('/', slash + 1);
			const std::string dir = root + "/" + dataDir.substr(0, slash);
			bool known = false;
			for (const std::string &existing : plan.mkdirs)
				known = known || (existing == dir);
			if (!known)
				plan.mkdirs.push_back(dir);
		} while (slash != dataDir.npos);
	}
}

// Flags, arguments and launch type of a launcher booted directly (not through
// nds-bootstrap-hb). autoBoostVram is what BOOST_VRAM=-1 means for this launch.
static void finishDirectPlan(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request, bool autoBoostVram)
{
	plan.boostCpu = (launcher.boostCpu != 0);
	plan.boostVram = boolSetting(launcher.boostVram, autoBoostVram);
	plan.dsModeSwitch = boolSetting(launcher.dsMode, !env.isDSiMode);
	plan.homebrewArg = addConfigArgs(plan, launcher, request);
	plan.launchType = LAUNCH_TYPE_CUSTOM;
	plan.ok = true;
}

static void planDirect(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request)
{
	if (launcher.resolve == LauncherResolve::BootDevice) {
		plan.ndsToBoot = launcher.binary.on(bootDevice(env));
		finishDirectPlan(plan, launcher, env, request, !env.isDSiMode);
	} else {
		bool onFat;
		plan.ndsToBoot = resolveSdFirst(launcher.binary, env, onFat);
		finishDirectPlan(plan, launcher, env, request, onFat);
	}
}

// FastVideoDS: the DLDI32 build for DLDI drivers of 32 KB or more (driverSize 0xF).
// From sd: that build is only chosen with a flashcard inserted, as before.
static void planFastVideo(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request)
{
	const LauncherBinary *dldi32 = launcher.variant("DLDI32");
	const bool bigDldi = (env.dldiDriverSize >= 0xF);

	const LauncherBinary &sdBinary = (env.flashcardFound && bigDldi && dldi32) ? *dldi32 : launcher.binary;
	const bool onFat = !(env.isDSiMode && sdBinary.onSd);
	if (onFat) {
		const LauncherBinary &fatBinary = (bigDldi && dldi32) ? *dldi32 : launcher.binary;
		plan.ndsToBoot = fatBinary.on("fat:");
	} else {
		plan.ndsToBoot = sdBinary.on("sd:");
	}
	finishDirectPlan(plan, launcher, env, request, onFat);
}

static void planS8dsColecoDS(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request)
{
	bool onFat;
	plan.ndsToBoot = resolveSdFirst(s8dsColecoDSBinary(launcher, env), env, onFat);
	finishDirectPlan(plan, launcher, env, request, onFat);
}

static bool endsWithNoCase(const std::string &str, const char *suffix)
{
	const size_t len = strlen(suffix);
	return str.size() > len && strncasecmp(str.c_str() + str.size() - len, suffix, len) == 0;
}

// An emulator booted from SD through nds-bootstrap-hb. nds-bootstrap-hb reads the
// emulator and its argument from nds-bootstrap.ini; with a RAM disk (romToRamDisk
// 0 = Genesis, 1 = SNES, 4 = PC Engine) it also loads the ROM into RAM itself.
static void planBootstrapHb(LaunchPlan &plan, const LauncherBinary &emulator, const LaunchRequest &request,
			    int romToRamDisk, const std::string &iniHomebrewArg, const std::string &iniRamDrivePath, bool boostCpu)
{
	plan.useNDSB = true;
	plan.romToRamDisk = romToRamDisk;
	plan.ndsToBoot = emulator.on("sd:");
	plan.ndsToBootFat = emulator.on("fat:");

	plan.writeBootstrapIni = true;
	plan.bootstrapHomebrewArg = iniHomebrewArg;
	plan.bootstrapRamDrivePath = iniRamDrivePath;
	plan.bootstrapBoostCpu = boostCpu ? 1 : 0;

	plan.boostCpu = boostCpu;
	plan.boostVram = false;
	plan.dsModeSwitch = false;
	for (const std::string &arg : request.prefixArgs)
		plan.args.push_back(arg);
	plan.args.push_back(plan.romPathFat);
	plan.homebrewArg = "";

	if (romToRamDisk == 0)
		plan.romIsCompressed = endsWithNoCase(plan.romPath, ".lz77.gen") || endsWithNoCase(plan.romPath, ".lz77.md");
	else if (romToRamDisk == 1)
		plan.romIsCompressed = endsWithNoCase(plan.romPath, ".lz77.smc") || endsWithNoCase(plan.romPath, ".lz77.sfc");
	else if (romToRamDisk == 4)
		plan.romIsCompressed = endsWithNoCase(plan.romPath, ".lz77.pce");

	plan.launchType = LAUNCH_TYPE_SD_FLASHCARD;
	plan.ok = true;
}

// NitroGrafx: through nds-bootstrap-hb with the ROM in RAM when Settings' "SMS/GG in RAM"
// option is on, the ROM is on SD and SCFG is unlocked; otherwise directly
static void planPce(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request)
{
	if (!env.secondaryDevice && !env.arm7SCFGLocked && env.smsGgInRam) {
		planBootstrapHb(plan, launcher.binary, request, 4, plan.romPath, "", true);
	} else {
		bool onFat;
		plan.ndsToBoot = resolveSdFirst(launcher.binary, env, onFat);
		finishDirectPlan(plan, launcher, env, request, onFat);
	}
}

// SNEmulDS, one of three ways:
// - Settings' new SNES emulator: the TGDS build (LAUNCHER_PATH_TWL) from sd: in DSi mode,
//   otherwise LAUNCHER_PATH from fat:, booted through TGDS multiboot as fat:/SNEmulDS.srl
// - ROM on a flashcard: the legacy build, or LAUNCHER_PATH_LEGACY_TWLM for ROMs in fat:/roms/snes
// - ROM on SD: the legacy build through nds-bootstrap-hb with the ROM in RAM
static void planSnes(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request)
{
	if (env.newSnesEmuVer) {
		const LauncherBinary *twl = launcher.variant("TWL");
		const bool onSd = (env.isDSiMode && twl && twl->onSd);
		plan.ndsToBoot = onSd ? twl->on("sd:") : launcher.binary.on("fat:");
		plan.tgdsMode = true;
		plan.tscTgds = true;
		plan.argv0 = "fat:/SNEmulDS.srl";
		plan.boostCpu = true;
		plan.boostVram = plan.dsModeSwitch = (!onSd && env.secondaryDevice);
		for (const std::string &arg : request.prefixArgs)
			plan.args.push_back(arg);
		plan.args.push_back(plan.romPathFat);	// TGDS only reaches the ROM through fat:
		plan.homebrewArg = plan.romPath;
		plan.launchType = LAUNCH_TYPE_CUSTOM;
		plan.ok = true;
	} else if (env.secondaryDevice) {
		const LauncherBinary *legacy = launcher.variant((request.romFolder == "fat:/roms/snes") ? "LEGACY_TWLM" : "LEGACY");
		if (!legacy)
			return;
		bool onFat;
		plan.ndsToBoot = resolveSdFirst(*legacy, env, onFat);
		plan.boostCpu = false;
		plan.boostVram = false;
		plan.dsModeSwitch = true;
		plan.homebrewArg = addConfigArgs(plan, launcher, request);
		plan.launchType = LAUNCH_TYPE_CUSTOM;
		plan.ok = true;
	} else {
		const LauncherBinary *legacy = launcher.variant("LEGACY");
		if (!legacy)
			return;
		planBootstrapHb(plan, *legacy, request, 1, "fat:/ROM.SMC", plan.romPath, false);
	}
}

// PicoDrive TWL or jEnesisDS (see genesisUsesPicoDrive()). PicoDrive TWL, and jEnesisDS for
// ROMs on a flashcard, boot directly, jEnesisDS in DS mode. jEnesisDS for ROMs on SD boots
// through nds-bootstrap-hb with the ROM in RAM.
static void planGenesis(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request)
{
	const bool usePicoDrive = genesisUsesPicoDrive(env, plan.romPath);
	if (usePicoDrive || env.secondaryDevice) {
		const LauncherBinary *pico = launcher.variant("PICO");
		if (usePicoDrive && !pico)
			return;
		bool onFat;
		plan.ndsToBoot = resolveSdFirst(usePicoDrive ? *pico : jEnesisBinary(launcher, env), env, onFat);
		finishDirectPlan(plan, launcher, env, request, onFat);
		plan.dsModeSwitch = !usePicoDrive;
	} else {
		planBootstrapHb(plan, jEnesisBinary(launcher, env), request, 0, "fat:/ROM.BIN", plan.romPath, true);
	}
}

// Directly booted GBA launchers: ARG as the argument, CPU boost, no DS mode switch
static void finishGbaPlan(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchRequest &request, bool boostVram, int launchType)
{
	plan.boostCpu = true;
	plan.boostVram = boostVram;
	plan.dsModeSwitch = false;
	plan.homebrewArg = addConfigArgs(plan, launcher, request);
	plan.launchType = launchType;
	plan.ok = true;
}

// GBA ROMs, the first of these that applies:
// - Settings' native mode with a Slot-2 cart: copied to the cart, then booted by gbapatcher (LAUNCHER_PATH_NATIVE)
// - GBARunner3 on the boot device
// - ROM on a flashcard: GBARunner2 for the console, directly
// - ROM on SD: GBARunner2 for the console, through nds-bootstrap-hb
static void planGba(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request)
{
	if (gbaUsesSlot2(env)) {
		const LauncherBinary *native = launcher.variant("NATIVE");
		if (!native)
			return;
		bool onFat;
		plan.ndsToBoot = resolveSdFirst(*native, env, onFat);
		plan.gbaNative = true;
		plan.gbaNativeKeepNor = request.resume;
		plan.gbaSavePath = plan.romPath.substr(0, plan.romPath.rfind('.')) + ".sav";
		finishGbaPlan(plan, launcher, request, false, LAUNCH_TYPE_GBA_NATIVE);
	} else if (const LauncherBinary *gbar3 = gbaRunner3(launcher, env)) {
		plan.ndsToBoot = gbar3->on(bootDevice(env));
		finishGbaPlan(plan, launcher, request, !env.isDSiMode, LAUNCH_TYPE_CUSTOM);
	} else if (env.secondaryDevice) {
		char tid[4] = {0};
		const bool pokemonEmerald = !env.dsiFeatures && env.readGbaTid
			&& env.readGbaTid(plan.romPath.c_str(), tid) && memcmp(tid, "BPE", 3) == 0;
		const LauncherBinary *runner = launcher.variant(gbaRunner2Name(env, false, pokemonEmerald));
		if (!runner)
			return;
		bool onFat;
		plan.ndsToBoot = resolveSdFirst(*runner, env, onFat);
		finishGbaPlan(plan, launcher, request, false, LAUNCH_TYPE_CUSTOM);
	} else {
		const LauncherBinary *runner = launcher.variant(gbaRunner2Name(env, true, false));
		if (!runner)
			return;
		planBootstrapHb(plan, *runner, request, -1, plan.romPath, "", true);
	}
}

// DSTWO plugins are booted by the DSTWO's own firmware, which reads the plugin to run
// from fat:/_dstwo/twlm.ini as a path without the "fat:" device. A plugin on SD is
// first copied to the DSTWO.
static void planDstwoPlugin(LaunchPlan &plan, const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request)
{
	bool onFat;
	plan.ndsToBoot = resolveSdFirst(launcher.binary, env, onFat);
	if (env.secondaryDevice) {
		plan.dstwoBootFile = plan.romPath.substr(plan.romPath.find(':') + 1);
	} else {
		plan.dstwoCopyTo = "fat:/_nds/TWiLightMenu/tempPlugin.plg";
		plan.dstwoBootFile = "/_nds/TWiLightMenu/tempPlugin.plg";
	}
	finishDirectPlan(plan, launcher, env, request, onFat);
}

LaunchPrecheck launcherPrecheck(const CustomLauncher *launcher, const LaunchEnv &env, const std::string &romPath)
{
	if (!launcher)
		return LaunchPrecheck::None;

	switch (launcher->handler) {
		case LauncherHandler::Gba:
			// Not for a Slot-2 cart on DS, which runs the ROM on the GBA's own hardware
			if (!env.secondaryDevice || env.dsiFeatures || env.gbaBooter == 2)
				return LaunchPrecheck::GbaBios;
			break;
		case LauncherHandler::Genesis:
			if (env.mdEmulator == 1 && env.fileSize && env.fileSize(romPath.c_str()) > 0x300000)
				return LaunchPrecheck::MdRomTooBig;
			break;
		case LauncherHandler::Snes:
		case LauncherHandler::Pce:
			if ((launcher->handler == LauncherHandler::Snes || env.smsGgInRam)
			 && env.isDSiMode && !env.dldiIsCycloDsi && env.arm7SCFGLocked)
				return LaunchPrecheck::LockedScfg;
			break;
		default:
			break;
	}
	return LaunchPrecheck::None;
}

LaunchPlan planLaunch(const CustomLauncher &launcher, const LaunchEnv &env, const LaunchRequest &request)
{
	LaunchPlan plan;
	plan.romPath = request.romFolder + "/" + request.filename;
	plan.romPathFat = toFat(plan.romPath);
	plan.ntrSdRelaunch = launcher.ntrSdRelaunch;

	if (!launcherAvailable(launcher, env))
		return plan;

	addDataDirs(plan, launcher, env);

	switch (launcher.handler) {
		case LauncherHandler::Direct:
			planDirect(plan, launcher, env, request);
			break;
		case LauncherHandler::FastVideo:
			planFastVideo(plan, launcher, env, request);
			break;
		case LauncherHandler::S8dsColecoDS:
			planS8dsColecoDS(plan, launcher, env, request);
			break;
		case LauncherHandler::DstwoPlugin:
			planDstwoPlugin(plan, launcher, env, request);
			break;
		case LauncherHandler::Pce:
			planPce(plan, launcher, env, request);
			break;
		case LauncherHandler::Snes:
			planSnes(plan, launcher, env, request);
			break;
		case LauncherHandler::Genesis:
			planGenesis(plan, launcher, env, request);
			break;
		case LauncherHandler::Gba:
			planGba(plan, launcher, env, request);
			break;
		default:
			break;
	}

	return plan;
}

bool launchTypeUsesLauncherConfig(int launchType, std::string_view romPath, bool secondaryDevice)
{
	if (isNdsFamilyFile(romPath) || !findCustomLauncher(romPath))
		return false;
	if (launchType == LAUNCH_TYPE_SD_FLASHCARD)
		return secondaryDevice;
	// 4 onwards: ENESDSLaunch up to ECustomLaunch, every launch type that isn't a DS game or DSiWare
	return launchType >= 4;
}
