#include "common/systemdetails.h"
#include "common/flashcard.h"
#include "common/arm7status.h"
#include "myDSiMode.h"

#include <nds/arm9/dldi.h>

#define CHECK_BIT(v, n) (((v) >> (n)) & 1)

#define FLASHM_BACKUPHEADER 0x3f680
#define FLASHME_MAJOR_VER 0x17c

// Make this link C-like for volatility
extern "C" {
	static volatile int _batteryLevel = 0;
	static volatile int _volumeLevel = -1;
	static volatile int _sdStatus = 0;

	void volBatSdCallback(u32 status, void *userdata) {
		_batteryLevel = (status & BAT_MASK) >> BAT_OFF;
		_volumeLevel = (status & VOL_MASK) >> VOL_OFF;
		_sdStatus = (status & SD_MASK) >> SD_OFF;
	}

	void registerFifoHandlers() {
		fifoSetValue32Handler(FIFO_USER_03, volBatSdCallback, NULL);
	}
}

SystemDetails::SystemDetails()
{

	_isRunFromSD = false;
	_dsiWramAccess = false;
	_dsiWramMirrored = false;
	_arm7SCFGLocked = false;
	_scfgSdmmcEnabled = false;
	_isRegularDS = true;
	_isDSPhat = false;
	_hasRegulableBacklight = true;
	_i2cBricked = false;
	_nitroFsInitOk = false;
	_fatInitOk = false;
	_fifoOk = false;

	fifoWaitValue32(FIFO_USER_03);

	const u32 status = ((fifoGetValue32(FIFO_USER_03)) >> INIT_OFF);

	if (isDSiMode()) {
		u32 wordBak = *(vu32*)0x03700000;
		*(vu32*)0x03700000 = 0x414C5253;
		_dsiWramAccess = *(vu32*)0x03700000 == 0x414C5253;
		_dsiWramMirrored = (*(vu32*)0x03700000 == 0x414C5253 && *(vu32*)0x03708000 == 0x414C5253);
		*(vu32*)0x03700000 = wordBak;
	}

	_arm7SCFGLocked = (*(u32*)0x02FFFDF0 == 0); // If TWiLight Menu++ is being run from DSiWarehax or flashcard, then arm7 SCFG is locked.

	_scfgSdmmcEnabled = (CHECK_BIT(status, SCFGSDMMC_BIT) != 0);

	if (CHECK_BIT(status, SNDEXTCNT_BIT) != 0) {
		_isRegularDS = false; // If sound frequency setting is found, then the console is not a DS Phat/Lite
	}

	_isDSPhat = CHECK_BIT(status, DSPHAT_BIT) != 0;

	_hasRegulableBacklight = CHECK_BIT(status, BACKLIGHT_BIT) != 0;

	_i2cBricked = CHECK_BIT(status, I2CBRICKED_BIT) != 0;

	if (!dsiFeatures()) {
		u32 wordBak = *(vu32*)0x02800000;
		u32 wordBak2 = *(vu32*)0x02C00000;
		*(vu32*)(0x02800000) = 0x314D454D;
		*(vu32*)(0x02C00000) = 0x324D454D;
		_dsDebugRam = ((*(vu32*)(0x02800000) == 0x314D454D) && (*(vu32*)(0x02C00000) == 0x324D454D));
		*(vu32*)(0x02800000) = wordBak;
		*(vu32*)(0x02C00000) = wordBak2;
	}

	// force is regular ds
	//_isRegularDS = true;
	// Restore value.
}

void SystemDetails::initArm7RegStatuses() {
	//printf("sys().initArm7RegStatuses()\n");
	if (!_fifoOk) {
		registerFifoHandlers();
	}
	_fifoOk = true;
}

int SystemDetails::batteryStatus() {
	return _batteryLevel;
}

int SystemDetails::volumeStatus() {
	return _volumeLevel;
}

SystemDetails::ESDStatus SystemDetails::sdStatus() {
	return (SystemDetails::ESDStatus)_sdStatus;
}

void SystemDetails::initFilesystem(const char *runningPath)
{
	extern const DISC_INTERFACE __my_io_dsisd;

	//printf("sys().initFilesystem()\n");
	if (_fatInitOk) {
		return;
	}

	*(u32*)(0x2FFFD0C) = 0x54494D52;	// Run reboot timer
	if (isDSiMode() && memcmp(io_dldi_data->friendlyName, "CycloDS iEvolution", 18) == 0) {
		*(u32*)(0x2FFFA04) = 0x49444C44;
		fatMountSimple("fat", &__my_io_dsisd);
		_fatInitOk = flashcardFound();
	} else {
		fatMountSimple("sd", &__my_io_dsisd);
		fatMountSimple("fat", dldiGetInternal());
		_fatInitOk = (sdFound() || flashcardFound());
	}
	*(u32*)(0x2FFFD0C) = 0;
	_isRunFromSD = (strncmp(runningPath, "sd:/", 4) == 0);
	chdir(_isRunFromSD ? "sd:/" : "fat:/");

	if (!_nitroFsInitOk && runningPath != NULL) {
		_nitroFsInitOk = nitroFSInit(runningPath) == 1;
	}
}
