
#include "dsimenusettings.h"
#include "bootstrappaths.h"
#include "systemdetails.h"
#include "common/inifile.h"
#include "flashcard.h"
#include <string.h>

DSiMenuPlusPlusSettings::DSiMenuPlusPlusSettings()
{
    
    romfolder[0] = "sd:/";
    romfolder[1] = "fat:/";

    pagenum[0] = 0;
    pagenum[1] = 0;

    cursorPosition[0] = 0;
    cursorPosition[1] = 0;

    startMenu_cursorPosition = 0;
    consoleModel = 0;

    gotosettings = false;

    guiLanguage = ELangDefault;
    colorMode = 0;
    blfLevel = 0;
    useGbarunner = false;
    showMainMenu = true;
    theme = 0;
    subtheme = 0;

	showNds = true;
	showNes = true;
	showGb = true;
	showSmsGg = true;
	showMd = true;
	showSnes = true;
    showDirectories = true;
    showHidden = false;
    showBoxArt = true;
    animateDsiIcons = true;
    sysRegion = -1;
    launcherApp = -1;
    secondaryAccess = false;
    previousUsedDevice = false;
    secondaryDevice = false;

    flashcard = EDSTTClone;

    slot1LaunchMethod = EDirect;

    useBootstrap = true;
    bootstrapFile = EReleaseBootstrap;

    bstrap_language = ELangDefault;
    boostCpu = false;
    boostVram = false;
    bstrap_dsiMode = EDSMode;
    slot1SCFGUnlock = false;

    show12hrClock = true;

    snesEmulator = true;

    ak_viewMode = EViewInternal;
    ak_scrollSpeed = EScrollFast;
    ak_theme = "zelda";
    ak_zoomIcons = true;

    launchType = ENoLaunch;
    homebrewBootstrap = EReleaseBootstrap;

    r4_theme = "unused";
    soundfreq = EFreq32KHz;
	dsiSplash = isDSiMode();
	hsMsg = false;
    showlogo = true;
    autorun = false;
}

void DSiMenuPlusPlusSettings::loadSettings()
{
    CIniFile settingsini(DSIMENUPP_INI);

    // UI settings.
    romfolder[0] = settingsini.GetString("SRLOADER", "ROM_FOLDER", romfolder[0]);
    romfolder[1] = settingsini.GetString("SRLOADER", "SECONDARY_ROM_FOLDER", romfolder[1]);

    pagenum[0] = settingsini.GetInt("SRLOADER", "PAGE_NUMBER", pagenum[0]);
    pagenum[1] = settingsini.GetInt("SRLOADER", "SECONDARY_PAGE_NUMBER", pagenum[1]);

    cursorPosition[0] = settingsini.GetInt("SRLOADER", "CURSOR_POSITION", cursorPosition[0]);
    cursorPosition[1] = settingsini.GetInt("SRLOADER", "SECONDARY_CURSOR_POSITION", cursorPosition[1]);

    startMenu_cursorPosition = settingsini.GetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", startMenu_cursorPosition);
    consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", consoleModel);

	showNds = settingsini.GetInt("SRLOADER", "SHOW_NDS", showNds);
	showNes = settingsini.GetInt("SRLOADER", "SHOW_NES", showNes);
	showGb = settingsini.GetInt("SRLOADER", "SHOW_GB", showGb);
	showSmsGg = settingsini.GetInt("SRLOADER", "SHOW_SMSGG", showSmsGg);
	showMd = settingsini.GetInt("SRLOADER", "SHOW_MDGEN", showMd);
	showSnes = settingsini.GetInt("SRLOADER", "SHOW_SNES", showSnes);

    // Customizable UI settings.
	colorMode = settingsini.GetInt("SRLOADER", "COLOR_MODE", colorMode);
	blfLevel = settingsini.GetInt("SRLOADER", "BLUE_LIGHT_FILTER_LEVEL", blfLevel);
    guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", guiLanguage);
    useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);
    if (!sys().isRegularDS()) {
        useGbarunner = true;
    }

	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", soundfreq);
    dsiSplash = settingsini.GetInt("SRLOADER", "DSI_SPLASH", dsiSplash);
    hsMsg = settingsini.GetInt("SRLOADER", "HS_MSG", hsMsg);
    showlogo = settingsini.GetInt("SRLOADER", "SHOWLOGO", showlogo);

	secondaryAccess = settingsini.GetInt("SRLOADER", "SECONDARY_ACCESS", secondaryAccess);
	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
   	romPath = settingsini.GetString("SRLOADER", "ROM_PATH", romPath);

    // secondaryDevice = settingsini.GetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
    // flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", flashcard);

    if (bothSDandFlashcard()) {
		secondaryDevice = settingsini.GetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
	} else if (flashcardFound()) {
		secondaryDevice = true;
	} else {
		secondaryDevice = false;
	}
    
    flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", 0);
    showMainMenu = settingsini.GetInt("SRLOADER", "SHOW_MAIN_MENU", showMainMenu);
    theme = settingsini.GetInt("SRLOADER", "THEME", theme);
    subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", subtheme);
    showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
    showHidden = settingsini.GetInt("SRLOADER", "SHOW_HIDDEN", showHidden);
    showBoxArt = settingsini.GetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
    animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);
	if (consoleModel < 2) {
		sysRegion = settingsini.GetInt("SRLOADER", "SYS_REGION", sysRegion);
		launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}


    slot1LaunchMethod = settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
    bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);
    if (!isDSiMode()) useBootstrap = settingsini.GetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);

    // Default nds-bootstrap settings
    boostCpu = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
    boostVram = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
    bstrap_dsiMode = settingsini.GetInt("NDS-BOOTSTRAP", "DSI_MODE", bstrap_dsiMode);
    slot1SCFGUnlock = settingsini.GetInt("SRLOADER", "SLOT1_SCFG_UNLOCK", slot1SCFGUnlock);

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

    snesEmulator = settingsini.GetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);

    autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", autorun);

}

void DSiMenuPlusPlusSettings::saveSettings()
{
    CIniFile settingsini(DSIMENUPP_INI);

    settingsini.SetString("SRLOADER", "ROM_FOLDER", romfolder[0]);
    settingsini.SetString("SRLOADER", "SECONDARY_ROM_FOLDER", romfolder[1]);
    
    settingsini.SetInt("SRLOADER", "PAGE_NUMBER", pagenum[0]);
    settingsini.SetInt("SRLOADER", "SECONDARY_PAGE_NUMBER", pagenum[1]);

    settingsini.SetInt("SRLOADER", "CURSOR_POSITION", cursorPosition[0]);
    settingsini.SetInt("SRLOADER", "SECONDARY_CURSOR_POSITION", cursorPosition[1]);

    settingsini.SetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", startMenu_cursorPosition);
    settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);

    // Customizable UI settings.
	settingsini.SetInt("SRLOADER", "COLOR_MODE", colorMode);
	settingsini.SetInt("SRLOADER", "BLUE_LIGHT_FILTER_LEVEL", blfLevel);
    settingsini.SetInt("SRLOADER", "LANGUAGE", guiLanguage);
    settingsini.SetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);

	settingsini.SetInt("SRLOADER", "SOUND_FREQ", soundfreq);
    settingsini.SetInt("SRLOADER", "DSI_SPLASH", dsiSplash);
    settingsini.SetInt("SRLOADER", "HS_MSG", hsMsg);
    settingsini.SetInt("SRLOADER", "SHOWLOGO", showlogo);

    settingsini.SetInt("SRLOADER", "SECONDARY_ACCESS", secondaryAccess);
    settingsini.SetInt("SRLOADER", "SHOW_MAIN_MENU", showMainMenu);
    settingsini.SetInt("SRLOADER", "THEME", theme);
    settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
	settingsini.SetInt("SRLOADER", "SHOW_NDS", showNds);
	settingsini.SetInt("SRLOADER", "SHOW_NES", showNes);
	settingsini.SetInt("SRLOADER", "SHOW_GB", showGb);
	settingsini.SetInt("SRLOADER", "SHOW_SMSGG", showSmsGg);
	settingsini.SetInt("SRLOADER", "SHOW_MDGEN", showMd);
	settingsini.SetInt("SRLOADER", "SHOW_SNES", showSnes);
    settingsini.SetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
    settingsini.SetInt("SRLOADER", "SHOW_HIDDEN", showHidden);
    settingsini.SetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
    settingsini.SetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);

	if (consoleModel < 2) {
		settingsini.SetInt("SRLOADER", "SYS_REGION", sysRegion);
		settingsini.SetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}

    settingsini.SetInt("SRLOADER", "FLASHCARD", flashcard);

    settingsini.SetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
    settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);
    if (!isDSiMode()) settingsini.SetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);

    // Default nds-bootstrap settings
    settingsini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", guiLanguage);
    settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
    settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
    settingsini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", bstrap_dsiMode);
    settingsini.SetInt("SRLOADER", "SLOT1_SCFG_UNLOCK", slot1SCFGUnlock);

    settingsini.SetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);

    settingsini.SetString("SRLOADER", "R4_THEME", r4_theme);
    settingsini.SetString("SRLOADER", "DSI_THEME", dsi_theme);

    settingsini.SetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);

    if (bothSDandFlashcard()) {
		settingsini.SetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
	}

    if (!gotosettings) {
		settingsini.SetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
		settingsini.SetString("SRLOADER", "ROM_PATH", romPath);
		settingsini.SetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
		settingsini.SetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
		settingsini.SetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
		settingsini.SetInt("SRLOADER", "LAUNCH_TYPE", launchType);
		settingsini.SetString("SRLOADER", "HOMEBREW_ARG", homebrewArg);
		settingsini.SetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);
	}
	

    settingsini.SaveIniFile(DSIMENUPP_INI);
}

DSiMenuPlusPlusSettings::TLanguage DSiMenuPlusPlusSettings::getGuiLanguage()
{
    if (guiLanguage == ELangDefault)
    {
        return (TLanguage)PersonalData->language;
    }
    return (TLanguage)guiLanguage;
}
