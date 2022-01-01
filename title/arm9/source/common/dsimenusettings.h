
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

    /*0: DSTT/R4i Gold/R4i-SDHC/R4 SDHC Dual-Core/R4 SDHC Upgrade/SC DSONE
    1: R4DS (Original Non-SDHC version)/ M3 Simply
    2: R4iDSN/R4i Gold RTS/R4 Ultra
    3: Acekard 2(i)/Galaxy Eagle/M3DS Real
    4: Acekard RPG
    5: Ace 3DS+/Gateway Blue Card/R4iTT
    6: SuperCard DSTWO
*/

    enum TFlashCard
    {
        EDSTTClone = 0,
        ER4Original = 1,
        ER4iGoldClone = 2,
        EAcekard2i = 3,
        EAcekardRPG = 4,
        EGatewayBlue = 5,
        ESupercardDSTWO = 6
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
    };

    enum TRegion
    {
        ERegionDefault = -1,
        ERegionJapan = 0,
        ERegionUSA = 1,
        ERegionEurope = 2,
        ERegionAustralia = 3,
        ERegionChina = 4,
        ERegionKorea = 5,
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
        EPicoDriveTWLLaunch = 10,
		EGBANativeLaunch = 11,
        EA7800DSLaunch = 12,
        EA5200DSLaunch = 13,
        ENitroGrafxLaunch = 14,
        EXEGSDSLaunch = 15,
        ENINTVDSLaunch = 16,
        EGBARunner2Launch = 17,
        EColecoDSLaunch = 18,
        ENitroSwanLaunch = 19,
        ENGPDSLaunch = 20
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

  public:
    TWLSettings();
    ~TWLSettings() {};

  public:
    void loadSettings();
    void saveSettings();

    TLanguage getGuiLanguage();
    std::string getGuiLanguageString();

    // Get if the current language is right to left
    bool rtl();

  public:
    std::string romfolder;
    int pagenum;
    int cursorPosition;
    int startMenu_cursorPosition;
    int consoleModel;
	bool languageSet;
	bool regionSet;
    int guiLanguage;
    int titleLanguage;
	bool macroMode;
    int colorMode;
    int blfLevel;
    int dsiWareExploit;
	bool gbar2DldiAccess;
    int wifiLed;
    bool useGbarunner;
    bool showMainMenu;
    int theme;
	int showGba;
	int showCol;
	int showSg;
    int showMd;
    bool showDirectories;
    int showBoxArt;
    bool animateDsiIcons;
    int sysRegion;
    int launcherApp;
    bool secondaryAccess;
    bool previousUsedDevice;
    bool secondaryDevice;
    bool fcSaveOnSd;

    int flashcard;
    int slot1LaunchMethod;
    bool useBootstrap;
    bool bootstrapFile;

    int gameLanguage;
    int gameRegion;
    bool useRomRegion;
	int extendedMemory;

	bool forceSleepPatch;
    bool slot1SCFGUnlock;
    int limitedMode;
	bool dsiWareBooter;
	bool dsiWareToSD;
    bool autorun;
	bool autostartSlot1;
    bool show12hrClock;

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
    std::string homebrewArg[2];
    bool homebrewBootstrap;
    bool homebrewHasWide;
    bool soundfreq;
    int dsiSplash;
    bool dsiSplashAutoSkip;
    int nintendoLogoColor;
    bool showlogo;
    std::string r4_theme;// unused...
    std::string unlaunchBg;
    std::string font;

    bool wideScreen;
    bool ignoreBlacklists;
};

typedef singleton<TWLSettings> menuSettings_s;
inline TWLSettings &ms() { return menuSettings_s::instance(); }

#endif //_DSIMENUPPSETTINGS_H_
