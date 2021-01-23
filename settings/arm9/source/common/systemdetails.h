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

  public:
    bool arm7SCFGLocked() { return _arm7SCFGLocked; }
    bool isRegularDS() { return true; }
    bool fatInitOk() { return _fatInitOk; }
    bool useNitroFS() { return _nitroFsInitOk; }
    void initFilesystem(const char *runningPath = NULL);

  private:
    bool _arm7SCFGLocked;
    bool _isRegularDS;
    bool _fatInitOk;
    bool _nitroFsInitOk;
};

typedef singleton<SystemDetails> systemDetails_s;

inline SystemDetails &sys() { return systemDetails_s::instance(); }
#endif