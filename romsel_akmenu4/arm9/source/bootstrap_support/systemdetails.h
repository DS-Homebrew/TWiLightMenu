#include "singleton.h"
#include <fat.h>
#include "nitrofs.h"
#pragma once
#ifndef __SYSTEM_DETAILS__
#define __SYSTEM_DETAILS__
class SystemDetails
{
    public:
        SystemDetails();
        ~SystemDetails() {}

    public:
        bool arm7SCFGLocked() { return _arm7SCFGLocked; }
        bool flashcardUsed() { return _flashcardUsed; }
        bool isRegularDS() { return _isRegularDS; }
        bool fatInitOk() { return _fatInitOk; }
        bool useNitroFS() { return _nitroFsInitOk; }
        inline void initFilesystem(const char* nitrofsPath, const char* runningPath = NULL) {
            if (_fatInitOk) return;

            _fatInitOk = fatInitDefault();
            _nitroFsInitOk = (bool)nitroFSInit(nitrofsPath);

            if (!_nitroFsInitOk && runningPath != NULL) {
                _nitroFsInitOk = (bool)nitroFSInit(runningPath);
            } else {
                _nitroFsInitOk = false;
            }
        }
    private:
        bool _arm7SCFGLocked;
        bool _flashcardUsed;
        bool _isRegularDS;
        bool _fatInitOk;
        bool _nitroFsInitOk;
};


typedef singleton<SystemDetails> systemDetails_s;

inline SystemDetails & sys() { return systemDetails_s::instance(); }
#endif