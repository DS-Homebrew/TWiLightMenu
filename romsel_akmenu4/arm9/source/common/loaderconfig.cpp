
#include "loaderconfig.h"
#include "tool/stringtool.h"
#include "common/inifile.h"
#include "nds_loader_arm9.h"
#include "tool/dbgtool.h"

LoaderConfig::LoaderConfig(const std::string& loaderPath,
                           const std::string& configPath) : _loaderPath(loaderPath), _configPath(configPath)
{
    dbg_printf("Loader Config OK");
}

LoaderConfig::~LoaderConfig() {

}

LoaderConfig& LoaderConfig::option(const std::string& section, const std::string& item, const std::string& value)
{
    _iniOptions.emplace_back(section, item, value);
    return *this;
}

LoaderConfig& LoaderConfig::option(const std::string& section, const std::string& item, int value)
{
    std::string strvalue = formatString("%d", value);
    _iniOptions.emplace_back(section, item, strvalue);
    return *this;
}

int LoaderConfig::launch(int argc, const char** argv, bool clearBrightness) 
{
    CIniFile file(_configPath);
    for (auto& [section, item, value] : _iniOptions) {
        file.SetString(section, item, value);
    }
    file.SaveIniFile(_configPath);
    return runNdsFile(_loaderPath.c_str(), argc, argv, clearBrightness);
}