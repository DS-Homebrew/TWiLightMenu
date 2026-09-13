/*
    launch/launchExecutor.cpp

    See launch/launchExecutor.h.
*/

#include "launch/launchExecutor.h"

#include <nds.h>
#include <nds/arm9/dldi.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

#include "common/bootstrappaths.h"
#include "common/flashcard.h"
#include "common/inifile.h"
#include "common/nds_bootstrap_loader.h"
#include "common/nds_loader_arm9.h"
#include "common/nitrofs.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "common/twlmenusettings.h"
#include "exptools.h"
#include "myDSiMode.h"

// The plan's launch types must match TWLSettings::TLaunchType
static_assert((int)LAUNCH_TYPE_SD_FLASHCARD == (int)TWLSettings::ESDFlashcardLaunch);
static_assert((int)LAUNCH_TYPE_GBA_NATIVE == (int)TWLSettings::EGBANativeLaunch);
static_assert((int)LAUNCH_TYPE_CUSTOM == (int)TWLSettings::ECustomLaunch);

// Defined by each binary that compiles universal/source/launch (source/fileCopy.cpp and main.cpp)
extern int fcopy(const char *sourcePath, const char *destinationPath);
extern char copyBuf[0x8000];
extern void s2RamAccess(bool open);
extern void gbaSramAccess(bool open);

static unsigned fileSizeOf(const char *path)
{
	struct stat st;
	if (stat(path, &st) != 0)
		return 0;
	return (unsigned)st.st_size;
}

static bool readGbaTid(const char *romPath, char tid[4])
{
	FILE *file = fopen(romPath, "rb");
	if (!file)
		return false;
	fseek(file, 0xAC, SEEK_SET);
	const bool read = (fread(tid, 1, 4, file) == 4);
	fclose(file);
	return read;
}

static const char *bootstrapHbPath(void)
{
	return ms().bootstrapFile ? "sd:/_nds/nds-bootstrap-hb-nightly.nds" : "sd:/_nds/nds-bootstrap-hb-release.nds";
}

LaunchEnv captureLaunchEnv(bool secondaryDevice)
{
	LaunchEnv env;

	env.isDSiMode = isDSiMode();
	env.dsiFeatures = dsiFeatures();
	env.arm7SCFGLocked = sys().arm7SCFGLocked();
	env.dsiWramAccess = sys().dsiWramAccess();
	env.sdFound = sdFound();
	env.flashcardFound = flashcardFound();
	env.isRunFromSD = sys().isRunFromSD();
	env.consoleModel = ms().consoleModel;
	env.dldiDriverSize = io_dldi_data->driverSize;
	env.dldiIsDstwo = (memcmp(io_dldi_data->friendlyName, "DSTWO(Slot-1)", 0xD) == 0);
	env.dldiIsCycloDsi = (memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) == 0);
	env.slot2Id = *(u16*)(0x020000C0);

	char currentDate[16];
	time_t raw;
	time(&raw);
	strftime(currentDate, sizeof(currentDate), "%m/%d", localtime(&raw));
	env.aprilFools = (strcmp(currentDate, "04/01") == 0);

	env.secondaryDevice = secondaryDevice;

	env.gbaBooter = ms().gbaBooter;
	env.colEmulator = ms().colEmulator;
	env.sgEmulator = ms().sgEmulator;
	env.mdEmulator = ms().mdEmulator;
	env.newSnesEmuVer = ms().newSnesEmuVer;
	env.smsGgInRam = ms().smsGgInRam;
	env.gbar2DldiAccess = ms().gbar2DldiAccess;
	env.macroMode = ms().macroMode;

	env.fileSize = fileSizeOf;
	env.readGbaTid = readGbaTid;

	return env;
}

// Copies the GBA ROM to the Slot-2 cart's RAM, or to an EZ-Flash's NOR flash, and up to
// 64 KB of its save to the cart's SRAM, for gbapatcher to boot
static void copyGbaToSlot2(const LaunchPlan &plan, const GbaNativeUi *ui)
{
	const u16 slot2Id = *(u16*)(0x020000C0);

	if (ui && ui->start) ui->start();

	char tid[4] = {0};
	readGbaTid(plan.romPath.c_str(), tid);
	const bool agbj = (strncmp(tid, "AGBJ", 4) == 0);

	u32 romSize = fileSizeOf(plan.romPath.c_str());
	u32 ptr = 0x08000000;
	if (agbj && romSize <= 0x40000) {
		ptr += 0x400;
	}
	if (romSize > 0x2000000) romSize = 0x2000000;
	const u32 barStep = (romSize / 192) ? (romSize / 192) : 1;

	bool nor = false;
	if (slot2Id == 0x5A45 && !agbj) {
		cExpansion::SetRompage(0);
		expansion().SetRampage(cExpansion::ENorPage);
		cExpansion::OpenNorWrite();
		cExpansion::SetSerialMode();
		if (!plan.gbaNativeKeepNor) {
			for (u32 address = 0; address < romSize && address < 0x2000000; address += 0x40000) {
				expansion().Block_Erase(address);
				if (ui && ui->progress) ui->progress((address+0x40000)/barStep);
			}
		}
		nor = true;
	} else if (slot2Id == 0x4353 && romSize > 0x1FFFFFE) {
		romSize = 0x1FFFFFE;
	}

	if (ui && ui->copying) ui->copying();

	if (!(nor && plan.gbaNativeKeepNor)) {
		FILE* gbaFile = fopen(plan.romPath.c_str(), "rb");
		if (gbaFile) {
			u32 curPtr = ptr;
			for (u32 len = romSize; len > 0; len -= (len>0x8000 ? 0x8000 : len)) {
				if (fread(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), gbaFile) > 0) {
					s2RamAccess(true);
					if (nor) {
						expansion().WriteNorFlash(curPtr-ptr, (u8*)copyBuf, (len>0x8000 ? 0x8000 : len));
					} else {
						tonccpy((u16*)curPtr, &copyBuf, (len>0x8000 ? 0x8000 : len));
					}
					s2RamAccess(false);
					curPtr += 0x8000;
					if (ui && ui->progress) ui->progress(((curPtr-ptr)+0x8000)/barStep);
				} else {
					break;
				}
			}
			fclose(gbaFile);
		}
	}

	u32 savesize = fileSizeOf(plan.gbaSavePath.c_str());
	if (savesize > 0x10000) savesize = 0x10000;

	if (savesize > 0) {
		FILE* savFile = fopen(plan.gbaSavePath.c_str(), "rb");
		if (savFile) {
			ptr = 0x0A000000;
			for (u32 len = savesize; len > 0; len -= (len>0x8000 ? 0x8000 : len)) {
				if (fread(&copyBuf, 1, (len>0x8000 ? 0x8000 : len), savFile) > 0) {
					gbaSramAccess(true);	// Switch to GBA SRAM
					cExpansion::WriteSram(ptr,(u8*)copyBuf,0x8000);
					gbaSramAccess(false);	// Switch out of GBA SRAM
					ptr += 0x8000;
				} else {
					break;
				}
			}
			fclose(savFile);
		}
	}

	if (ui && ui->finish) ui->finish();
}

void applyLaunchSideEffects(const LaunchPlan &plan, const GbaNativeUi *gbaNativeUi)
{
	if (plan.gbaNative)
		copyGbaToSlot2(plan, gbaNativeUi);

	for (const std::string &dir : plan.mkdirs)
		mkdir(dir.c_str(), 0777);

	if (plan.writeBootstrapIni) {
		CIniFile bootstrapini(BOOTSTRAP_INI);

		bootstrapini.SetString("NDS-BOOTSTRAP", "GUI_LANGUAGE", ms().getGuiLanguageString());
		bootstrapini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", ms().gameLanguage);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", 0);
		bootstrapini.SetString("NDS-BOOTSTRAP", "NDS_PATH", plan.ndsToBoot);
		bootstrapini.SetString("NDS-BOOTSTRAP", "HOMEBREW_ARG", plan.bootstrapHomebrewArg);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", plan.bootstrapBoostCpu);
		bootstrapini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", 0);
		bootstrapini.SetString("NDS-BOOTSTRAP", "RAM_DRIVE_PATH", plan.bootstrapRamDrivePath);

		bootstrapini.SaveIniFile(BOOTSTRAP_INI);
	}

	if (!plan.dstwoBootFile.empty()) {
		if (!plan.dstwoCopyTo.empty()) {
			fcopy(plan.romPath.c_str(), plan.dstwoCopyTo.c_str());
		}

		CIniFile dstwobootini("fat:/_dstwo/twlm.ini");
		dstwobootini.SetString("boot_settings", "file", plan.dstwoBootFile);
		dstwobootini.SaveIniFile("fat:/_dstwo/twlm.ini");
	}
}

void applyPlanToSettings(const LaunchPlan &plan, bool secondaryDevice)
{
	ms().romPath[secondaryDevice] = plan.romPath;
	ms().previousUsedDevice = secondaryDevice;
	ms().homebrewBootstrap = true;
	if (plan.launchType != LAUNCH_TYPE_UNCHANGED)
		ms().launchType[secondaryDevice] = (TWLSettings::TLaunchType)plan.launchType;
	ms().homebrewArg[secondaryDevice] = plan.homebrewArg;
}

void prepareLaunch(const LaunchPlan &plan, bool bootstrapDirect)
{
	if (bootstrapDirect && plan.useNDSB) {
		bootFSInit(bootstrapHbPath());
		bootstrapHbRunPrep(plan.romToRamDisk);
	}
}

int runLaunch(const LaunchPlan &plan, bool bootstrapDirect)
{
	const char *ndsToBoot = (plan.useNDSB && !bootstrapDirect) ? bootstrapHbPath() : plan.ndsToBoot.c_str();

	std::vector<const char *> argv;
	argv.push_back(plan.tgdsMode ? plan.argv0.c_str() : ndsToBoot);
	for (const std::string &arg : plan.args)
		argv.push_back(arg.c_str());

	if (bootstrapDirect && plan.useNDSB) {
		if (access(bootstrapHbPath(), F_OK) != 0)
			return 1;

		char gameTid[5] = {0};
		u16 headerCRC = 0;
		FILE *ndsFile = fopen(plan.ndsToBoot.c_str(), "rb");
		if (ndsFile) {
			fseek(ndsFile, 0xC, SEEK_SET);
			fread(gameTid, 1, 4, ndsFile);
			fseek(ndsFile, 0x15E, SEEK_SET);
			fread(&headerCRC, sizeof(u16), 1, ndsFile);
			fclose(ndsFile);
		}
		if (gameTid[0] == 0)
			memset(gameTid, '#', 4); // Fix blank TID

		char patchOffsetCacheFilePath[64];
		snprintf(patchOffsetCacheFilePath, sizeof(patchOffsetCacheFilePath), "sd:/_nds/nds-bootstrap/patchOffsetCache/%s-%04X.bin", gameTid, headerCRC);

		const bool useRamDisk = (plan.romToRamDisk != -1);
		return bootstrapHbRunNdsFile(plan.ndsToBoot.c_str(), plan.ndsToBootFat.c_str(),
				useRamDisk ? plan.romPath.c_str() : "",
				"sd:/snemulds.cfg",
				useRamDisk ? fileSizeOf(plan.romPath.c_str()) : 0,
				"sd:/_nds/nds-bootstrap/softResetParams.bin",
				patchOffsetCacheFilePath,
				fileSizeOf("sd:/snemulds.cfg"),
				plan.romToRamDisk,
				plan.romIsCompressed,
				argv.size(),
				&argv[0],
				ms().gameLanguage,
				0,
				plan.boostCpu,
				plan.boostVram,
				ms().consoleModel, ms().soundFreq, false);
	}

	return runNdsFile(ndsToBoot, argv.size(), &argv[0], sys().isRunFromSD(), !plan.useNDSB, true,
			plan.dsModeSwitch, plan.boostCpu, plan.boostVram, plan.tscTgds, -1);
}
