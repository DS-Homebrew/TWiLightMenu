
#include <nds.h>
#include <string>
#include <vector>
#include <tuple>
#include <functional>

#include "bootstrappaths.h"

#pragma once
#ifndef __BOOTSTRAP_CONFIG__
#define __BOOTSTRAP_CONFIG__

enum APFixType {
    ENone = 0,
    EHasIPS = 1,
    EMissingIPS = 2,
    ERGFPatch = 3,
};

class BootstrapConfig
{
    public:
        BootstrapConfig(const std::string& fileName, const std::string& fullPath, const u8* gametid, u32 sdkVersion, u16 headerCrc16, int heapShrink);

        ~BootstrapConfig();
        
        BootstrapConfig& donorSdk(int sdk);
        BootstrapConfig& mpuSize(int mpuSize);
        BootstrapConfig& mpuRegion(int mpuRegion);
        BootstrapConfig& ceCached(bool ceCached);
        BootstrapConfig& saveSize(int saveSize);
        BootstrapConfig& cheatData(const std::string& cheatData);

        BootstrapConfig& donorSdk();
        BootstrapConfig& mpuSettings();
        BootstrapConfig& speedBumpExclude(int heapShrink);
        BootstrapConfig& forceSleepPatch();
        BootstrapConfig& saveSize();

        BootstrapConfig& dsiMode(int dsiMode);
        BootstrapConfig& vramBoost(bool vramBoost);
        BootstrapConfig& cpuBoost(bool cpuBoost);

        BootstrapConfig& language(int language);
        BootstrapConfig& saveNo(int saveNo);
        BootstrapConfig& ramDiskNo(int ramDiskNo);
        BootstrapConfig& nightlyBootstrap(bool nightlyBootstrap);
        BootstrapConfig& wideScreen(bool wideScreen);
        BootstrapConfig& gbarBootstrap(bool gbarBootstrap);

        BootstrapConfig& onWidescreenFailed(std::function<void(std::string)> handler);
        BootstrapConfig& onBeforeSaveCreate(std::function<void(void)> handler);
        BootstrapConfig& onSaveCreated(std::function<void(void)> handler);
        BootstrapConfig& onConfigSaved(std::function<void(void)> handler);
        BootstrapConfig& onCheatsApplied(std::function<void(void)> handler);

        APFixType hasAPFix();
        bool checkCompatibility();
        int launch();
    private:
        std::string getAPFix();
        std::string createSaveFileIfNotExists();
        void createTmpFileIfNotExists();
        void loadCheats();

        const std::string _fileName;
        const std::string _fullPath;
        const std::string _gametid;
        const u32 _sdkVersion;
        const u16 _headerCrc16;

        std::function<void(std::string)>  _widescreenFailedHandler;
        std::function<void(void)> _saveBeforeCreatedHandler;
        std::function<void(void)> _saveCreatedHandler;
        std::function<void(void)>  _configSavedHandler;
        std::function<void(void)>  _cheatsAppliedHandler;

        bool _useWideScreen;
        bool _useNightlyBootstrap;
        bool _useGbarBootstrap;
        int _donorSdk;
        int _mpuSize;
        int _mpuRegion;
		bool _ceCached;
        int _forceSleepPatch;
        bool _isHomebrew;
        int _saveSize;
        int _dsiMode;
        bool _vramBoost;
        bool _cpuBoost;
        int _language;
        int _saveNo;
        int _ramDiskNo;
        bool _soundFix;
        std::string _cheatData;
        std::string _apFix;
};
#endif