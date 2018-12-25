
#include <nds.h>
#include <string>
#include <vector>
#include <tuple>
#include <functional>

#include "bootstrappaths.h"

#pragma once
#ifndef __BOOTSTRAP_CONFIG__
#define __BOOTSTRAP_CONFIG__

class BootstrapConfig
{
    public:
        BootstrapConfig(const std::string& fileName, const std::string& gametid, u32 sdkVersion);

        ~BootstrapConfig();
        
        BootstrapConfig& donorSdk(int sdk);
        BootstrapConfig& mpuSize(int mpuSize);
        BootstrapConfig& mpuRegion(int mpuRegion);
        BootstrapConfig& saveSize(int saveSize);

        BootstrapConfig& donorSdk();
        BootstrapConfig& mpuSettings();
        BootstrapConfig& saveSize();
        BootstrapConfig& softReset();

        BootstrapConfig& dsiMode(bool dsiMode);
        BootstrapConfig& vramBoost(bool vramBoost);
        BootstrapConfig& cpuBoost(bool cpuBoost);

        BootstrapConfig& language(int language);
        BootstrapConfig& softReset(bool softReset);
        BootstrapConfig& soundFix(bool soundFix);
        BootstrapConfig& nightlyBootstrap(bool nightlyBootstrap);

        BootstrapConfig& onSaveCreated(std::function<void(void)> handler);
        BootstrapConfig& onConfigSaved(std::function<void(void)> handler);
        BootstrapConfig& onCheatsApplied(std::function<void(void)> handler);

        int launch();



    private:

        void createSaveFileIfNotExists();
        void createTmpFileIfNotExists();

        const std::string _fileName;
        const std::string _gametid;
        const u32 _sdkVersion;

        std::function<void(void)> _saveCreatedHandler;
        std::function<void(void)>  _configSavedHandler;
        std::function<void(void)>  _cheatsAppliedHandler;

        bool _useNightlyBootstrap;
        int _donorSdk;
        int _mpuSize;
        int _mpuRegion;
        bool _isHomebrew;
        int _saveSize;
        bool _dsiMode;
        bool _vramBoost;
        bool _cpuBoost;
        int _language;
        bool _softReset;
        bool _soundFix;
};
#endif