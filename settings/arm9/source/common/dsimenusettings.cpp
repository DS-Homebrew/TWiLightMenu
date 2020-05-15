
#include "dsimenusettings.h"
#include "bootstrappaths.h"
#include "systemdetails.h"
#include "common/inifile.h"
#include <string.h>

DSiMenuPlusPlusSettings::DSiMenuPlusPlusSettings()
{
    romfolder = "";
    pagenum = 0;
    cursorPosition = 0;
    startMenu_cursorPosition = 0;
    consoleModel = 0;

    guiLanguage = ELangDefault;
	titleLanguage = -1;
    colorMode = 0;
    blfLevel = 0;
    dsiWareExploit = 0;
    wifiLed = true;
    sdRemoveDetect = true;
    showMicroSd = false;
    useGbarunner = false;
	gbar2DldiAccess = false;
    showMainMenu = false;
    showSelectMenu = false;
    theme = 0;
    subtheme = 0;
    dsiMusic = 1;

	showNds = true;
	showRvid = true;
	showA26 = true;
	showNes = true;
	showGb = true;
	showSmsGg = true;
	showMd = 3;
	showSnes = true;
	updateRecentlyPlayedList = true;
    sortMethod = 0;
    showDirectories = true;
    showHidden = false;
    showBoxArt = true;
    cacheBoxArt = true;
    animateDsiIcons = true;
    sysRegion = -1;
    launcherApp = -1;
    secondaryAccess = false;
    previousUsedDevice = false;
    secondaryDevice = false;
	fcSaveOnSd = false;

    flashcard = EDSTTClone;

    slot1LaunchMethod = EDirect;

    useBootstrap = isDSiMode();
    bootstrapFile = EReleaseBootstrap;

    bstrap_language = ELangDefault;
    boostCpu = false;
    boostVram = false;
    bstrap_dsiMode = EDSMode;
	forceSleepPatch = false;
    slot1SCFGUnlock = false;
    dsiWareBooter = false;

    show12hrClock = true;

    //snesEmulator = true;
    smsGgInRam = false;

    ak_viewMode = EViewInternal;
    ak_scrollSpeed = EScrollFast;
    ak_theme = "zelda";
    ak_zoomIcons = true;

    launchType = ENoLaunch;
    homebrewBootstrap = EReleaseBootstrap;

    r4_theme = "unused";
    
    dsi_theme = "dark";
    _3ds_theme = "light";
    
    soundFreq = EFreq32KHz;
	dsiSplash = isDSiMode();
	hsMsg = false;
    showlogo = true;
    autorun = false;

    //screenScaleSize = 0;
	wideScreen = false;
}

void DSiMenuPlusPlusSettings::loadSettings()
{
    CIniFile settingsini(DSIMENUPP_INI);

    // UI settings.
    romfolder = settingsini.GetString("SRLOADER", "ROM_FOLDER", romfolder);

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

    // Customizable UI settings.
	colorMode = settingsini.GetInt("SRLOADER", "COLOR_MODE", colorMode);
	blfLevel = settingsini.GetInt("SRLOADER", "BLUE_LIGHT_FILTER_LEVEL", blfLevel);
	dsiWareExploit = settingsini.GetInt("SRLOADER", "DSIWARE_EXPLOIT", dsiWareExploit);
	wifiLed = settingsini.GetInt("SRLOADER", "WIFI_LED", wifiLed);
    guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", guiLanguage);
	titleLanguage = settingsini.GetInt("SRLOADER", "TITLELANGUAGE", titleLanguage);
    sdRemoveDetect = settingsini.GetInt("SRLOADER", "SD_REMOVE_DETECT", sdRemoveDetect);
    showMicroSd = settingsini.GetInt("SRLOADER", "SHOW_MICROSD", showMicroSd);
    useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);
    if (!sys().isRegularDS()) {
        useGbarunner = true;
    }
    gbar2DldiAccess = settingsini.GetInt("SRLOADER", "GBAR2_DLDI_ACCESS", gbar2DldiAccess);

    dsiSplash = settingsini.GetInt("SRLOADER", "DSI_SPLASH", dsiSplash);
    hsMsg = settingsini.GetInt("SRLOADER", "HS_MSG", hsMsg);
    showlogo = settingsini.GetInt("SRLOADER", "SHOWLOGO", showlogo);

	secondaryAccess = settingsini.GetInt("SRLOADER", "SECONDARY_ACCESS", secondaryAccess);
	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
    secondaryDevice = settingsini.GetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
	fcSaveOnSd = settingsini.GetInt("SRLOADER", "FC_SAVE_ON_SD", fcSaveOnSd);

    showMainMenu = settingsini.GetInt("SRLOADER", "SHOW_MAIN_MENU", showMainMenu);
    showSelectMenu = settingsini.GetInt("SRLOADER", "SHOW_SELECT_MENU", showSelectMenu);
    theme = settingsini.GetInt("SRLOADER", "THEME", theme);
    subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", subtheme);
    dsiMusic = settingsini.GetInt("SRLOADER", "DSI_MUSIC", dsiMusic);
    updateRecentlyPlayedList = settingsini.GetInt("SRLOADER", "UPDATE_RECENTLY_PLAYED_LIST", updateRecentlyPlayedList);
    sortMethod = settingsini.GetInt("SRLOADER", "SORT_METHOD", sortMethod);
    showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
    showHidden = settingsini.GetInt("SRLOADER", "SHOW_HIDDEN", showHidden);
    preventDeletion = settingsini.GetInt("SRLOADER", "PREVENT_ROM_DELETION", preventDeletion);
    showBoxArt = settingsini.GetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
    cacheBoxArt = settingsini.GetInt("SRLOADER", "CACHE_BOX_ART", cacheBoxArt);
    animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);
	if (consoleModel < 2) {
		sysRegion = settingsini.GetInt("SRLOADER", "SYS_REGION", sysRegion);
		launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}

    flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", flashcard);

    slot1LaunchMethod = settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
    bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);
    useBootstrap = settingsini.GetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);

    // Default nds-bootstrap settings
    boostCpu = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
    boostVram = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
    bstrap_dsiMode = settingsini.GetInt("NDS-BOOTSTRAP", "DSI_MODE", bstrap_dsiMode);
    forceSleepPatch = settingsini.GetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", forceSleepPatch);
	soundFreq = settingsini.GetInt("NDS-BOOTSTRAP", "SOUND_FREQ", soundFreq);
    slot1SCFGUnlock = settingsini.GetInt("SRLOADER", "SLOT1_SCFG_UNLOCK", slot1SCFGUnlock);
	dsiWareBooter = settingsini.GetInt("SRLOADER", "DSIWARE_BOOTER", dsiWareBooter);

    ak_viewMode = settingsini.GetInt("SRLOADER", "AK_VIEWMODE", ak_viewMode);
    ak_scrollSpeed = settingsini.GetInt("SRLOADER", "AK_SCROLLSPEED", ak_scrollSpeed);
    ak_theme = settingsini.GetString("SRLOADER", "AK_THEME", ak_theme);
    ak_zoomIcons = settingsini.GetInt("SRLOADER", "AK_ZOOM_ICONS", ak_zoomIcons);

    dsiWareSrlPath = settingsini.GetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
    dsiWarePubPath = settingsini.GetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
    dsiWarePrvPath = settingsini.GetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
    launchType = settingsini.GetInt("SRLOADER", "LAUNCH_TYPE", launchType);
    homebrewArg = settingsini.GetString("SRLOADER", "HOMEBREW_ARG", homebrewArg);
    homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);

    show12hrClock = settingsini.GetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);

    r4_theme = settingsini.GetString("SRLOADER", "R4_THEME", r4_theme);
    dsi_theme = settingsini.GetString("SRLOADER", "DSI_THEME", dsi_theme);
    _3ds_theme = settingsini.GetString("SRLOADER", "3DS_THEME", _3ds_theme);

    //snesEmulator = settingsini.GetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);
    smsGgInRam = settingsini.GetInt("SRLOADER", "SMS_GG_IN_RAM", smsGgInRam);

    autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", autorun);

    //screenScaleSize = settingsini.GetInt("TWL_FIRM", "SCREENSCALESIZE", screenScaleSize);
    wideScreen = settingsini.GetInt("SRLOADER", "WIDESCREEN", wideScreen);
}

void DSiMenuPlusPlusSettings::saveSettings()
{
    CIniFile settingsini(DSIMENUPP_INI);

    settingsini.SetString("SRLOADER", "ROM_FOLDER", romfolder);

    settingsini.SetInt("SRLOADER", "PAGE_NUMBER", pagenum);
    settingsini.SetInt("SRLOADER", "CURSOR_POSITION", cursorPosition);
    settingsini.SetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", startMenu_cursorPosition);
    settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);

    // Customizable UI settings.
	settingsini.SetInt("SRLOADER", "COLOR_MODE", colorMode);
	settingsini.SetInt("SRLOADER", "BLUE_LIGHT_FILTER_LEVEL", blfLevel);
	settingsini.SetInt("SRLOADER", "DSIWARE_EXPLOIT", dsiWareExploit);
	settingsini.SetInt("SRLOADER", "WIFI_LED", wifiLed);
    settingsini.SetInt("SRLOADER", "LANGUAGE", guiLanguage);
	settingsini.SetInt("SRLOADER", "TITLELANGUAGE", titleLanguage);
    settingsini.SetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);
    settingsini.SetInt("SRLOADER", "GBAR2_DLDI_ACCESS", gbar2DldiAccess);
	settingsini.SetInt("SRLOADER", "SD_REMOVE_DETECT", sdRemoveDetect);
	settingsini.SetInt("SRLOADER", "SHOW_MICROSD", showMicroSd);

    settingsini.SetInt("SRLOADER", "DSI_SPLASH", dsiSplash);
    settingsini.SetInt("SRLOADER", "HS_MSG", hsMsg);
    settingsini.SetInt("SRLOADER", "SHOWLOGO", showlogo);

    settingsini.SetInt("SRLOADER", "SECONDARY_ACCESS", secondaryAccess);
    settingsini.SetInt("SRLOADER", "SHOW_MAIN_MENU", showMainMenu);
    settingsini.SetInt("SRLOADER", "SHOW_SELECT_MENU", showSelectMenu);
    settingsini.SetInt("SRLOADER", "THEME", theme);
    settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
    settingsini.SetInt("SRLOADER", "DSI_MUSIC", dsiMusic);
	settingsini.SetInt("SRLOADER", "SHOW_NDS", showNds);
	settingsini.SetInt("SRLOADER", "SHOW_RVID", showRvid);
	settingsini.SetInt("SRLOADER", "SHOW_A26", showA26);
	settingsini.SetInt("SRLOADER", "SHOW_NES", showNes);
	settingsini.SetInt("SRLOADER", "SHOW_GB", showGb);
	settingsini.SetInt("SRLOADER", "SHOW_SMSGG", showSmsGg);
	settingsini.SetInt("SRLOADER", "SHOW_MDGEN", showMd);
	settingsini.SetInt("SRLOADER", "SHOW_SNES", showSnes);
    settingsini.SetInt("SRLOADER", "UPDATE_RECENTLY_PLAYED_LIST", updateRecentlyPlayedList);
    settingsini.SetInt("SRLOADER", "SORT_METHOD", sortMethod);
    settingsini.SetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
    settingsini.SetInt("SRLOADER", "SHOW_HIDDEN", showHidden);
    settingsini.SetInt("SRLOADER", "PREVENT_ROM_DELETION", preventDeletion);
    settingsini.SetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
    settingsini.SetInt("SRLOADER", "CACHE_BOX_ART", cacheBoxArt);
    settingsini.SetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);
	if (consoleModel < 2) {
		settingsini.SetInt("SRLOADER", "SYS_REGION", sysRegion);
		settingsini.SetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}

    settingsini.SetInt("SRLOADER", "FLASHCARD", flashcard);

    settingsini.SetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
    settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);
    settingsini.SetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);
	settingsini.SetInt("SRLOADER", "FC_SAVE_ON_SD", fcSaveOnSd);

    // Default nds-bootstrap settings
    settingsini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", guiLanguage);
    settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
    settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
    settingsini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", bstrap_dsiMode);
    settingsini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", forceSleepPatch);
	settingsini.SetInt("NDS-BOOTSTRAP", "SOUND_FREQ", soundFreq);
    settingsini.SetInt("SRLOADER", "SLOT1_SCFG_UNLOCK", slot1SCFGUnlock);
	settingsini.SetInt("SRLOADER", "DSIWARE_BOOTER", dsiWareBooter);

    settingsini.SetInt("SRLOADER", "AK_VIEWMODE", ak_viewMode);
    settingsini.SetInt("SRLOADER", "AK_SCROLLSPEED", ak_scrollSpeed);
    settingsini.SetString("SRLOADER", "AK_THEME", ak_theme);
    settingsini.SetInt("SRLOADER", "AK_ZOOM_ICONS", ak_zoomIcons);

    settingsini.SetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);

    settingsini.SetString("SRLOADER", "R4_THEME", r4_theme);
    settingsini.SetString("SRLOADER", "DSI_THEME", dsi_theme);
    settingsini.SetString("SRLOADER", "3DS_THEME", _3ds_theme);

    //settingsini.SetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);
	settingsini.SetInt("SRLOADER", "SMS_GG_IN_RAM", smsGgInRam);

    //settingsini.SetInt("TWL_FIRM", "SCREENSCALESIZE", screenScaleSize);
    settingsini.SetInt("SRLOADER", "WIDESCREEN", wideScreen);

    settingsini.SaveIniFile(DSIMENUPP_INI);
}

DSiMenuPlusPlusSettings::TLanguage DSiMenuPlusPlusSettings::getGuiLanguage()
{
    if (guiLanguage == ELangDefault)
    {
        extern bool useTwlCfg;
        return (TLanguage)(useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
    }
    return (TLanguage)guiLanguage;
}
