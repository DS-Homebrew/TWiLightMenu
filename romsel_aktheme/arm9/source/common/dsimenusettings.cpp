
#include "dsimenusettings.h"
#include "bootstrappaths.h"
#include "systemdetails.h"
#include "common/inifile.h"
#include "common/flashcard.h"
#include <string.h>

static const char* settingsinipath = DSIMENUPP_INI;

const char *charUnlaunchBg;
bool removeLauncherPatches = false;

TWLSettings::TWLSettings()
{
    romfolder[0] = "sd:/";
    romfolder[1] = "fat:/";
    pagenum = 0;
    cursorPosition = 0;
    startMenu_cursorPosition = 0;
    consoleModel = 0;

    gotosettings = false;
    guiLanguage = ELangDefault;
    titleLanguage = -1;
    useGbarunner = false;
    gbar2DldiAccess = false;
    showMicroSd = false;
    theme = 0;

    showNds = true;
    showRvid = true;
    showA26 = true;
    showNes = true;
    showGb = true;
    showSmsGg = true;
    showMd = 3;
    showSnes = true;
    showPce = true;
    hideEmptyBoxes = false;
    showDirectories = true;
    showHidden = false;
    showBoxArt = 1 + isDSiMode();
    animateDsiIcons = true;
    preventDeletion = false;
    launcherApp = -1;
    previousUsedDevice = false;
    secondaryDevice = false;
    fcSaveOnSd = false;

    slot1LaunchMethod = EReboot;

    useBootstrap = isDSiMode();
    bootstrapFile = EReleaseBootstrap;

    gameLanguage = ELangDefault;
    forceSleepPatch = false;
    dsiWareBooter = false;

    show12hrClock = true;

    //snesEmulator = true;
    smsGgInRam = false;

    ak_viewMode = EViewInternal;
    ak_scrollSpeed = EScrollFast;
    ak_theme = "zelda";
    ak_zoomIcons = true;

    slot1Launched = false;
    launchType[0] = ENoLaunch;
    launchType[1] = ENoLaunch;
    homebrewBootstrap = EReleaseBootstrap;
    homebrewHasWide = false;

    r4_theme = "unused";
    gbaBorder = "default.png";
    unlaunchBg = "default.gif";

    soundfreq = EFreq32KHz;
    showlogo = true;
    autorun = false;

    wideScreen = false;
}

void TWLSettings::loadSettings()
{
    if (access(settingsinipath, F_OK) != 0 && flashcardFound()) {
        settingsinipath = DSIMENUPP_INI_FC;		// Fallback to .ini path on flashcard, if not found on SD card, or if SD access is disabled
    }

    CIniFile settingsini(settingsinipath);

    // UI settings.
    romfolder[0] = settingsini.GetString("SRLOADER", "ROM_FOLDER", romfolder[0]);
    romfolder[1] = settingsini.GetString("SRLOADER", "SECONDARY_ROM_FOLDER", romfolder[1]);
	if (strncmp(romfolder[0].c_str(), "sd:", 3) != 0) {
		romfolder[0] = "sd:/";
	}
	if (strncmp(romfolder[1].c_str(), "fat:", 4) != 0) {
		romfolder[1] = "fat:/";
	}

    pagenum = settingsini.GetInt("SRLOADER", "PAGE_NUMBER", pagenum);
    cursorPosition = settingsini.GetInt("SRLOADER", "CURSOR_POSITION", cursorPosition);
    startMenu_cursorPosition = settingsini.GetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", startMenu_cursorPosition);
    consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", consoleModel);

    showNds = settingsini.GetInt("SRLOADER", "SHOW_NDS", showNds);
    showRvid = settingsini.GetInt("SRLOADER", "SHOW_RVID", showRvid);
    showA26 = settingsini.GetInt("SRLOADER", "SHOW_A26", showA26);
    showNes = settingsini.GetInt("SRLOADER", "SHOW_NES", showNes);
    showGb = settingsini.GetInt("SRLOADER", "SHOW_GB", showGb);
    showSmsGg = settingsini.GetInt("SRLOADER", "SHOW_SMSGG", showSmsGg);
    showMd = settingsini.GetInt("SRLOADER", "SHOW_MDGEN", showMd);
    showSnes = settingsini.GetInt("SRLOADER", "SHOW_SNES", showSnes);
    showPce = settingsini.GetInt("SRLOADER", "SHOW_PCE", showPce);

    // Customizable UI settings.
    guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", guiLanguage);
    titleLanguage = settingsini.GetInt("SRLOADER", "TITLELANGUAGE", titleLanguage);
    useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);
    if (!sys().isRegularDS()) {
        useGbarunner = true;
    }
    gbar2DldiAccess = settingsini.GetInt("SRLOADER", "GBAR2_DLDI_ACCESS", gbar2DldiAccess);
    showMicroSd = settingsini.GetInt("SRLOADER", "SHOW_MICROSD", showMicroSd);

    soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", soundfreq);
    showlogo = settingsini.GetInt("SRLOADER", "SHOWLOGO", showlogo);

    previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
	secondaryDevice = bothSDandFlashcard() ? settingsini.GetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice) : flashcardFound();
    fcSaveOnSd = settingsini.GetInt("SRLOADER", "FC_SAVE_ON_SD", fcSaveOnSd);

    theme = settingsini.GetInt("SRLOADER", "THEME", theme);
    hideEmptyBoxes = settingsini.GetInt("SRLOADER", "HIDE_EMPTY_BOXES", hideEmptyBoxes);
    showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
    showHidden = settingsini.GetInt("SRLOADER", "SHOW_HIDDEN", showHidden);
    showBoxArt = settingsini.GetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
    animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);
    guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", guiLanguage);
    if (consoleModel < 2) {
        sysRegion = settingsini.GetInt("SRLOADER", "SYS_REGION", sysRegion);
        launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
    }

    slot1LaunchMethod = settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
    bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);
    useBootstrap = settingsini.GetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);

    // Default nds-bootstrap settings
    gameLanguage = settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);
    forceSleepPatch = settingsini.GetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", forceSleepPatch);
    dsiWareBooter = settingsini.GetInt("SRLOADER", "DSIWARE_BOOTER", dsiWareBooter);

    ak_viewMode = settingsini.GetInt("SRLOADER", "AK_VIEWMODE", ak_viewMode);
    ak_scrollSpeed = settingsini.GetInt("SRLOADER", "AK_SCROLLSPEED", ak_scrollSpeed);
    ak_theme = settingsini.GetString("SRLOADER", "AK_THEME", ak_theme);
    ak_zoomIcons = settingsini.GetInt("SRLOADER", "AK_ZOOM_ICONS", ak_zoomIcons);

    dsiWareSrlPath = settingsini.GetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
    dsiWarePubPath = settingsini.GetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
    dsiWarePrvPath = settingsini.GetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
    launchType[0] = settingsini.GetInt("SRLOADER", "LAUNCH_TYPE", launchType[0]);
    launchType[1] = settingsini.GetInt("SRLOADER", "SECONDARY_LAUNCH_TYPE", launchType[1]);
    romPath[0] = settingsini.GetString("SRLOADER", "ROM_PATH", romPath[0]);
    romPath[1] = settingsini.GetString("SRLOADER", "SECONDARY_ROM_PATH", romPath[1]);
	if (strncmp(romPath[0].c_str(), "sd:", 3) != 0) {
		romPath[0] = "";
	}
	if (strncmp(romPath[1].c_str(), "fat:", 4) != 0) {
		romPath[1] = "";
	}

    homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);

    unlaunchBg = settingsini.GetString("SRLOADER", "UNLAUNCH_BG", unlaunchBg);
	gbaBorder = settingsini.GetString("SRLOADER", "GBA_BORDER", gbaBorder);
	charUnlaunchBg = unlaunchBg.c_str();
	removeLauncherPatches = settingsini.GetInt("SRLOADER", "UNLAUNCH_PATCH_REMOVE", removeLauncherPatches);

    show12hrClock = settingsini.GetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);

    //snesEmulator = settingsini.GetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);
    smsGgInRam = settingsini.GetInt("SRLOADER", "SMS_GG_IN_RAM", smsGgInRam);

    autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", autorun);
    
    wideScreen = settingsini.GetInt("SRLOADER", "WIDESCREEN", wideScreen);
}

void TWLSettings::saveSettings()
{
    CIniFile settingsini(settingsinipath);

    settingsini.SetString("SRLOADER", "ROM_FOLDER", romfolder[0]);
    settingsini.SetString("SRLOADER", "SECONDARY_ROM_FOLDER", romfolder[1]);

    settingsini.SetInt("SRLOADER", "PAGE_NUMBER", pagenum);
    settingsini.SetInt("SRLOADER", "CURSOR_POSITION", cursorPosition);
    settingsini.SetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", startMenu_cursorPosition);
    settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
    // Customizable UI settings.
    settingsini.SetInt("SRLOADER", "LANGUAGE", guiLanguage);
    settingsini.SetInt("SRLOADER", "TITLELANGUAGE", titleLanguage);
    settingsini.SetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);

    if (bothSDandFlashcard()) {
        settingsini.SetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
    }
    settingsini.SetInt("SRLOADER", "THEME", theme);
    settingsini.SetInt("SRLOADER", "HIDE_EMPTY_BOXES", hideEmptyBoxes);
    settingsini.SetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
    settingsini.SetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
    settingsini.SetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);

    settingsini.SetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
    settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);
    if (!isDSiMode()) settingsini.SetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);

    // Default nds-bootstrap settings
    //settingsini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);

    settingsini.SetInt("SRLOADER", "AK_VIEWMODE", ak_viewMode);
    settingsini.SetInt("SRLOADER", "AK_SCROLLSPEED", ak_scrollSpeed);
    settingsini.SetString("SRLOADER", "AK_THEME", ak_theme);
    settingsini.SetInt("SRLOADER", "AK_ZOOM_ICONS", ak_zoomIcons);

    if (!gotosettings) {
        settingsini.SetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
        settingsini.SetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
        settingsini.SetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
        settingsini.SetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
        settingsini.SetInt("SRLOADER", "LAUNCH_TYPE", launchType[0]);
        settingsini.SetInt("SRLOADER", "SECONDARY_LAUNCH_TYPE", launchType[1]);
        settingsini.SetString("SRLOADER", "ROM_PATH", romPath[0]);
        settingsini.SetString("SRLOADER", "SECONDARY_ROM_PATH", romPath[1]);
        settingsini.SetString("SRLOADER", ms().secondaryDevice ? "SECONDARY_HOMEBREW_ARG" : "HOMEBREW_ARG", homebrewArg);
        settingsini.SetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);
        settingsini.SetInt("SRLOADER", "HOMEBREW_HAS_WIDE", homebrewHasWide);
    }

    settingsini.SetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);

    settingsini.SaveIniFile(DSIMENUPP_INI);
}

u32 TWLSettings::CopyBufferSize(void)
{
//   if (font().FontRAM()<300*1024) return 1024*1024;
//   return 512*1024;
return 0x8000;
}

TWLSettings::TLanguage TWLSettings::getGuiLanguage()
{
    if (guiLanguage == ELangDefault)
    {
        extern bool useTwlCfg;
        return (TLanguage)(useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
    }
    return (TLanguage)guiLanguage;
}

std::string TWLSettings::getGuiLanguageString()
{
    switch (getGuiLanguage())
    {
        case TWLSettings::ELangJapanese:
            return "ja";
        case TWLSettings::ELangEnglish:
        default:
            return "en";
        case TWLSettings::ELangFrench:
            return "fr";
        case TWLSettings::ELangGerman:
            return "de";
        case TWLSettings::ELangItalian:
            return "it";
        case TWLSettings::ELangSpanish:
            return "es";
        case TWLSettings::ELangChineseS:
            return "zh-CN";
        case TWLSettings::ELangKorean:
            return "ko";
        case TWLSettings::ELangChineseT:
            return "zh-TW";
        case TWLSettings::ELangPolish:
            return "pl";
        case TWLSettings::ELangPortuguese:
            return "pt";
        case TWLSettings::ELangRussian:
            return "ru";
        case TWLSettings::ELangSwedish:
            return "sv";
        case TWLSettings::ELangDanish:
            return "da";
        case TWLSettings::ELangTurkish:
            return "tr";
        case TWLSettings::ELangUkrainian:
            return "uk";
        case TWLSettings::ELangHungarian:
            return "hu";
        case TWLSettings::ELangNorwegian:
            return "no";
        case TWLSettings::ELangHebrew:
            return "he";
        case TWLSettings::ELangDutch:
            return "nl";
        case TWLSettings::ELangIndonesian:
            return "id";
        case TWLSettings::ELangGreek:
            return "el";
        case TWLSettings::ELangBulgarian:
            return "bg";
        case TWLSettings::ELangRomanian:
            return "ro";
        case TWLSettings::ELangArabic:
            return "ar";
        case TWLSettings::ELangPortugueseBrazil:
            return "pt-BR";
        case TWLSettings::ELangVietnamese:
            return "vi";
        case TWLSettings::ELangValencian:
            return "val";
        case TWLSettings::ELangCatalan:
            return "ca";
        case TWLSettings::ELangRyukyuan:
            return "ry";
        case TWLSettings::ELangCzech:
            return "cs";
        case TWLSettings::ELangFinnish:
            return "fi";
    }
}
