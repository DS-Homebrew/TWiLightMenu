
#include <nds.h>
#include <string>
#include <vector>
#include <tuple>

#pragma once
#ifndef __LOADER_CONFIG__
#define __LOADER_CONFIG__

class LoaderConfig
{
    public:
        LoaderConfig(const std::string& loaderPath,
             const std::string& configPath);

        ~LoaderConfig();
        
        LoaderConfig& option(const std::string& section, const std::string& item, const std::string& value);
        LoaderConfig& option(const std::string& section, const std::string& item, int value);
        
        int launch(int argc = 0, const char** argv = NULL, bool dldiPatchNds = true, bool clearBrightness = true, bool dsModeSwitch = false, bool boostCpu = true, bool boostVram = true);

    private:
        const std::string _loaderPath;
        const std::string _configPath;
        std::vector<std::tuple<std::string, std::string, std::string>> _iniOptions;
};
#endif