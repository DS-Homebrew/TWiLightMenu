#pragma once
#ifndef __SYSTEM_DETAILS__
#define __SYSTEM_DETAILS__
#include "common/singleton.h"
#include <fat.h>
#include <stdio.h>
#include "nitrofs.h"
#include <nds.h>

class SystemDetails
{
public:
	SystemDetails();
	~SystemDetails() {}

	enum ESDStatus {
		SDOk = 0,
		SDEjected = 1,
		SDInserted = 2 
	};

	bool dsiWramAccess() { return _dsiWramAccess; }
	bool dsiWramMirrored() { return _dsiWramMirrored; }
	bool arm7SCFGLocked() { return _arm7SCFGLocked; }
	bool scfgSdmmcEnabled() { return _scfgSdmmcEnabled; }
	bool isRunFromSD() { return _isRunFromSD; }
	bool isRegularDS() { return _isRegularDS; }
	bool fatInitOk() { return _fatInitOk; }
	bool useNitroFS() { return _nitroFsInitOk; }
	bool isDSPhat() { return (_isRegularDS && _isDSPhat); }
	bool isDSLite() { return (_isRegularDS && !_isDSPhat); }
	bool hasRegulableBacklight() { return _hasRegulableBacklight; }
	bool i2cBricked() { return _i2cBricked; }
	bool dsDebugRam() { return _dsDebugRam; }
	void initFilesystem(const char *runningPath);
	void initArm7RegStatuses();
	int batteryStatus();
	int volumeStatus();
	ESDStatus sdStatus();

private:
	bool _dsiWramAccess;
	bool _dsiWramMirrored;
	bool _arm7SCFGLocked;
	bool _scfgSdmmcEnabled;
	bool _isRunFromSD;
	bool _isRegularDS;
	bool _isDSPhat;
	bool _hasRegulableBacklight;
	bool _i2cBricked;
	bool _dsDebugRam;
	bool _fatInitOk;
	bool _fifoOk;
	bool _nitroFsInitOk;
};

typedef singleton<SystemDetails> systemDetails_s;

inline SystemDetails &sys() { return systemDetails_s::instance(); }
#endif
