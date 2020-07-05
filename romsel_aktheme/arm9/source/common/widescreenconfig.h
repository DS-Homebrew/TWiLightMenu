
#include <string>
#include <nds.h>

#pragma once
#ifndef __WIDESCREEN_CONFIG__
#define __WIDESCREEN_CONFIG__
class WidescreenConfig {
    public:
        WidescreenConfig();
        WidescreenConfig(const std::string& filename);
        WidescreenConfig& gamePatch(const char* gameTid, u16 gameCrc);
        WidescreenConfig& isHomebrew(bool homebrew);
        WidescreenConfig& enable(bool enable);
        const std::string apply();
        ~WidescreenConfig(){}
    private:
        std::string _widescreenPatch;
        bool _isHomebrew;
        bool _useWidescreen;
};
#endif