
#include <nds.h>
#include <string>
#include <vector>
#include "common/bootstrappaths.h"
#include "common/singleton.h"

#pragma once
#ifndef _DSIMENUPPSETTINGS_H_
#define _DSIMENUPPSETTINGS_H_

/**
 * Multi use class for TWiLight Menu++ INI file.
 * 
 * Try not to change settings that are not related to the current theme.
 */
class TWLSettings
{
public:
	const char *settingsinipath = DSIMENUPP_INI;

	// enum TScrollSpeed : int
	// {
	// 	EScrollFast = 4,
	// 	EScrollMedium = 10,
	// 	EScrollSlow = 16
	// };

	enum TViewMode : int
	{
		EViewList = 0,
		EViewIcon = 1,
		EViewInternal = 2,
		EViewSmallIcon = 3
	};

	// Do not reorder these, just add to the end
	enum TLanguage : int
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
		ELangKazakh = 32,
		ELangGalician = 33,
	};

	enum TRegion : int
	{
		ERegionDefault = -1,
		ERegionJapan = 0,
		ERegionUSA = 1,
		ERegionEurope = 2,
		ERegionAustralia = 3,
		ERegionChina = 4,
		ERegionKorea = 5,
	};

	enum TRunIn : int
	{
		EDSMode = 0,
		EAutoDSiMode = 1,
		EDSiMode = 2
	};

	enum TSlot1LaunchMethod : int
	{
		EReboot = 0,
		EDirect = 1,
		EUnlaunch = 2
	};

	enum TBootstrapFile : bool
	{
		EReleaseBootstrap = false,
		ENightlyBootstrap = true
	};

	enum TLaunchType : int
	{
		ENoLaunch = 0,
		ESDFlashcardLaunch = 1,
		ESDFlashcardDirectLaunch = 2,
		EDSiWareLaunch = 3,
		ENESDSLaunch = 4,
		EGameYobLaunch = 5,
		ES8DSLaunch = 6,
		ERVideoLaunch = 7,
		EFastVideoLaunch = 8,
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
		ENGPDSLaunch = 20,
		ESNEmulDSLaunch = 21,
		ESugarDSLaunch = 22,
		ESugarDSLaunch2 = 23, // Duplicated, as two CPC emulators we're bundled prior to including SugarDS
		ETunaViDSLaunch = 24,
		EImageLaunch = 25,
		E3DSLaunch = 26,
		EPokeMiniLaunch = 27
	};

	enum TConsoleModel : int
	{
		EDSiRetail = 0, // Nintendo DSi (Retail)
		EDSiDebug = 1, // Nintendo DSi (Dev/Panda)
		E3DSOriginal = 2, // Nintendo 3DS
		E3DSNew = 3 // New Nintendo 3DS
	};

	enum TExploit : int
	{
		EExploitNone = 0,
		EExploitSudokuhax = 1,
		EExploit4Swordshax = 2,
		EExploitFieldrunnerhax = 3,
		EExploitGrtpwn = 4,
		EExploitUgopwn = 5,
		EExploitUnopwn = 6,
		EExploitMemoryPit = 7
	};

	enum TTheme : int
	{
		EThemeDSi = 0,
		ETheme3DS = 1,
		EThemeR4 = 2,
		EThemeWood = 3,
		EThemeSaturn = 4,
		EThemeHBL = 5,
		EThemeGBC = 6
	};

	enum TDSiMusic : int
	{
		EMusicOff = 0,
		EMusicRegular = 1,
		EMusicShop = 2,
		EMusicTheme = 3,
		EMusicRegular3D = 4,
		EMusicHBL = 5
	};

	enum TSettingsMusic : int
	{
		ESMusicTheme = -1,
		ESMusicOff = 0,
		ESMusicDSi = 1,
		ESMusic3DS = 2
	};

	enum TDSiWareBooter : bool
	{
		EDSiWareUnlaunch = false,
		EDSiWareBootstrap = true
	};

	enum TGbaBooter : int
	{
		EGbaNativeGbar2 = 1,
		EGbaGbar2 = 2
	};

	enum TColSegaEmulator : int
	{
		EColSegaS8DS = 1,
		EColSegaColecoDS = 2
	};

	enum TMegaDriveEmulator : int
	{
		EMegaDriveJenesis = 1,
		EMegaDrivePico = 2,
		EMegaDriveHybrid = 3
	};

	enum TSortMethod : int
	{
		ESortAlphabetical = 0,
		ESortRecent = 1,
		ESortMostPlayed = 2,
		ESortFileType = 3,
		ESortCustom = 4
	};

	enum TSoundFreq : bool
	{
		EFreq32KHz = false,
		EFreq47KHz = true
	};

	enum TSaveLoc : int
	{
		ESavesFolder = 0,
		EGamesFolder = 1,
		ETWLMFolder = 2
	};

	enum TSaveRelocation : bool
	{
		ERelocOnSDCard = true,
		ELocaOnNDSCart = false
	};

public:
	TWLSettings();
	~TWLSettings() {};

public:
	void loadSettings();
	void saveSettings();

	TLanguage getGuiLanguage();
	TLanguage getGameLanguage();
	TLanguage getTitleLanguage();

	std::string getGuiLanguageString();

	// Get if the current language is right to left
	bool rtl();

	TRegion getGameRegion();

	const char* getAppName();
public:
	std::string romfolder[2];
	std::string defaultRomfolder[2];
	std::string romPath[2];

	bool cursorAlwaysAtStart;
	int pagenum[2];
	int cursorPosition[2];
	int saveCursorPosition[2];

	TConsoleModel consoleModel;
	bool languageSet;
	bool regionSet;

	bool logging;
	TLanguage guiLanguage, currentLanguage;
	TLanguage titleLanguage;
	bool macroMode;
	bool sleepMode;
	bool kioskMode;
	TExploit dsiWareExploit;
	bool wifiLed;
	int wifiLedVer;
	bool powerLedColor;
	bool sdRemoveDetect;
	bool showMicroSd;
	bool gbar2DldiAccess;
	bool showMainMenu;
	bool showSelectMenu;
	bool rocketRobzLogo;
	TTheme theme;
	TSettingsMusic settingsMusic;
	TDSiMusic dsiMusic;
	bool boxArtColorDeband;

	TGbaBooter gbaBooter;
	bool gbaR3Test;
	TColSegaEmulator colEmulator;
	TColSegaEmulator sgEmulator;
	TMegaDriveEmulator mdEmulator;
	//int snesEmulator;
	bool updateRecentlyPlayedList;
	TSortMethod sortMethod;
	bool hideEmptyBoxes;
	bool showDirectories;
	bool showHidden;
	bool showPhoto;
	int showBoxArt;
	int filenameDisplay;
	bool animateDsiIcons;
	bool showCustomIcons;
	bool preventDeletion;
	TRegion sysRegion;
	int launcherApp;
	bool secondaryAccess;
	bool previousUsedDevice;
	bool secondaryDevice;
	TSaveLoc saveLocation;
	std::vector<std::string> blockedExtensions;

	TSlot1LaunchMethod slot1LaunchMethod;

	int dsiSplash;
	bool oppositeSplash;
	bool dsiSplashEasterEggs;
	bool dsiSplashAutoSkip;
	int nintendoLogoColor;
	bool showlogo;
	bool longSplashJingle;
	bool autorun;
	bool autostartSlot1;

	bool show12hrClock;

	std::string r4_theme;
	std::string dsi_theme;
	std::string _3ds_theme;
	std::string gbaBorder;
	std::string unlaunchBg;
	int removeLauncherPatches;
	std::string font;
	bool useThemeFont;
	bool dsClassicCustomFont;

	bool dontShowClusterWarning;
	bool ignoreBlacklists;

	bool slot1AccessSD;
	bool slot1SCFGUnlock;
	bool ezFlashRam;
	int limitedMode;
	bool dontShowDSiWareInDSModeWarning;
	TDSiWareBooter dsiWareBooter;
	bool dsiWareToSD;
	bool newSnesEmuVer;
	bool smsGgInRam;
	bool esrbRatingScreen;

	int ak_viewMode;
	// int ak_scrollSpeed;
	bool ak_zoomIcons;
	std::string ak_theme;

	bool useBootstrap;
	bool btsrpBootloaderDirect;
	TBootstrapFile bootstrapFile;
	bool kernelUseable;

	std::string internetBrowserPath;
	bool internetBrowserLaunched;
	std::string dsiWareSrlPath;
	std::string dsiWarePubPath;
	std::string dsiWarePrvPath;
	bool slot1Launched;
	TLaunchType launchType[2];
	std::string homebrewArg[2];
	bool homebrewBootstrap;
	bool homebrewHasWide;

	//int screenScaleSize;
	bool wideScreen;

	TLanguage gameLanguage;
	TRegion gameRegion;
	bool useRomRegion;
	bool boostCpuForClut;
	bool forceSleepPatch;
	TSoundFreq soundFreq;
	TSaveRelocation saveRelocation;
};

typedef singleton<TWLSettings> menuSettings_s;
inline TWLSettings &ms() { return menuSettings_s::instance(); }

#endif //_DSIMENUPPSETTINGS_H_
