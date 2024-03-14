
#include <nds.h>
#include <string>
#include "common/singleton.h"
#include "defaultSettings.h"

#pragma once
#ifndef _DSIMENUPPSETTINGS_H_
#define _DSIMENUPPSETTINGS_H_

/**
 * Multi use class for DSiMenuPlusPlus INI file.
 * 
 * Try not to change settings that are not related to the current theme.
 */
class TWLSettings
{
  public:
    enum TScrollSpeed
    {
        EScrollFast = 4,
        EScrollMedium = 10,
        EScrollSlow = 16
    };

    enum TViewMode
    {
        EViewList = 0,
        EViewIcon = 1,
        EViewInternal = 2
    };

    // Do not reorder these, just add to the end
    enum TLanguage
    {
        ELangDefault = -1,
        ELangJapanese = 0,
        ELangEnglish = 1,
        ELangFrench = 2,
        ELangGerman = 3,
        ELangItalian = 4,
        ELangSpanish = 5,
        ELangChineseS = 6,
        ELangKorean = 7,
        ELangChineseT = 8,
        ELangPolish = 9,
        ELangPortuguese = 10,
        ELangRussian = 11,
        ELangSwedish = 12,
        ELangDanish = 13,
        ELangTurkish = 14,
        ELangUkrainian = 15,
        ELangHungarian = 16,
        ELangNorwegian = 17,
        ELangHebrew = 18,
        ELangDutch = 19,
        ELangIndonesian = 20,
        ELangGreek = 21,
        ELangBulgarian = 22,
        ELangRomanian = 23,
        ELangArabic = 24,
        ELangPortugueseBrazil = 25,
        ELangVietnamese = 26,
        ELangValencian = 27,
        ELangCatalan = 28,
        ELangRyukyuan = 29,
        ELangCzech = 30,
        ELangFinnish = 31,
    };

    enum TRunIn
    {
        EDSMode = 0,
        EAutoDSiMode = 1,
        EDSiMode = 2
    };

    enum TSlot1LaunchMethod
    {
        EReboot = 0,
        EDirect = 1
    };

    enum TBootstrapFile
    {
        EReleaseBootstrap = false,
        ENightlyBootstrap = true
    };

    // 0 = No launch, 1 = SD/Flash card, 2 = SD/Flash card (Direct boot), 3 = DSiWare, 4 = NES, 5 = (S)GB(C), 6 = SMS/GG
    enum TLaunchType
    {
        ENoLaunch = 0,
        ESDFlashcardLaunch = 1,
        ESDFlashcardDirectLaunch = 2,
        EDSiWareLaunch = 3,
        ENESDSLaunch = 4,
        EGameYobLaunch = 5,
        ES8DSLaunch = 6,
        ERVideoLaunch = 7,
        EMPEG4Launch = 8,
        EStellaDSLaunch = 9,
        EPicoDriveTWLLaunch = 10
    };

    /*	0 = Nintendo DSi (Retail)
    1 = Nintendo DSi (Dev/Panda)
    2 = Nintendo 3DS
    3 = New Nintendo 3DS	*/
    enum TConsoleModel
    {
        EDSiRetail = 0,
        EDSiDebug = 1,
        E3DSOriginal = 2,
        E3DSNew = 3
    };

    enum TSoundFreq
    {
        EFreq32KHz = 0,
        EFreq47KHz = 1
    };

  public:
    TWLSettings();
    ~TWLSettings();

  public:
    void loadSettings();
    void saveSettings();
    static u32 CopyBufferSize(void);

    TLanguage getGuiLanguage();

    std::string getGuiLanguageString();

    const char* getAppName();
  public:
    std::string romfolder[2];
    int pagenum;
    int cursorPosition;
    int startMenu_cursorPosition;
    int consoleModel;
    int guiLanguage;
    int titleLanguage;
    bool useGbarunner;
    bool gbar2DldiAccess;
    bool showMicroSd;
    int theme;
    bool showNds;
    bool showRvid;
    bool showA26;
    bool showNes;
    bool showGb;
    bool showSmsGg;
    int showMd;
    bool showSnes;
    bool showPce;
    bool hideEmptyBoxes;
    bool showDirectories;
    bool showHidden;
    bool preventDeletion;
    int showBoxArt;
    bool animateDsiIcons;
    int sysRegion;
    int launcherApp;
    bool gotosettings;
    bool previousUsedDevice;
    bool secondaryDevice;
    bool fcSaveOnSd;

    int slot1LaunchMethod;
    bool useBootstrap;
    bool bootstrapFile;

    int gameLanguage;
    bool forceSleepPatch;
    bool dsiWareBooter;
    bool autorun;
    bool show12hrClock;

    //bool snesEmulator;
    bool smsGgInRam;

    int ak_viewMode;
    int ak_scrollSpeed;
    bool ak_zoomIcons;
    std::string ak_theme;

    std::string dsiWareSrlPath;
    std::string dsiWarePubPath;
    std::string dsiWarePrvPath;

    bool slot1Launched;
    int launchType[2];
    std::string romPath[2];
    std::string homebrewArg;
    bool homebrewBootstrap;
    bool homebrewHasWide;
    bool soundfreq;
    bool showlogo;
    std::string r4_theme;// unused...
	std::string gbaBorder;
    std::string unlaunchBg;

    bool wideScreen;
};

typedef singleton<TWLSettings> menuSettings_s;
inline TWLSettings &ms() { return menuSettings_s::instance(); }

#endif //_DSIMENUPPSETTINGS_H_
