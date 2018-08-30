#include "singleton.h"

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
    private:
        bool _arm7SCFGLocked;
        bool _flashcardUsed;
        bool _isRegularDS;
};


typedef singleton<SystemDetails> systemDetails_s;

inline SystemDetails & sys() { return systemDetails_s::instance(); }
#endif