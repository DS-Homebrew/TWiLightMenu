#pragma once
#ifndef __UNLAUNCH_BOOT_H__
#define __UNLAUNCH_BOOT_H__
#include <nds.h>
#include <string>
#include <functional>

#define BOOTTHIS_SRL "sd:/bootthis.dsi"
#define BOOTTHIS_PUB "sd:/bootthis.pub"
#define BOOTTHIS_PRV "sd:/bootthis.prv"

class UnlaunchBoot
{
    public:
        UnlaunchBoot(const std::string& fileName, u32 pubSavSize, u32 prvSavSize);
        ~UnlaunchBoot(){}
        bool prepare();
        void launch();
        UnlaunchBoot& onPubSavCreated(std::function<void(void)> handler);
        UnlaunchBoot& onPrvSavCreated(std::function<void(void)> handler);

    private: 
        bool doRenames(const std::string &fileExt);
        void createSaveIfNotExists(const std::string& fileExt, const std::string& savExt, u32 saveSize);
        std::function<void(void)>  _pubSavCreatedHandler;
        std::function<void(void)>  _prvSavCreatedHandler;
        std::string _fileName;
        u32 _pubSavSize;
        u32 _prvSavSize;
};

#endif