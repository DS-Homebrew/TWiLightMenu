
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
    appName = ENameTWiLightMenu;

    guiLanguage = ELangDefault;
    useGbarunner = false;
    theme = 0;
    subtheme = 0;

    showDirectories = true;
    showBoxArt = true;
    animateDsiIcons = true;
    sysRegion = -1;
    launcherApp = -1;
    secondaryAccess = false;
    previousUsedDevice = false;
    secondaryDevice = false;

    flashcard = EDSTTClone;

    slot1LaunchMethod = EDirect;

    bootstrapFile = EReleaseBootstrap;

    bstrap_language = ELangDefault;
    boostCpu = false;
    boostVram = false;
    soundFix = false;
    bstrap_dsiMode = false;

    show12hrClock = true;

    ak_viewMode = EViewInternal;
    ak_scrollSpeed = EScrollFast;
    ak_theme = "zelda";
    ak_zoomIcons = true;

    launchType = ENoLaunch;
    homebrewBootstrap = EReleaseBootstrap;

    r4_theme = "unused";
    soundfreq = EFreq32KHz;
    showlogo = true;
    autorun = false;
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
    appName = settingsini.GetInt("SRLOADER", "APP_NAME", appName);

    // Customizable UI settings.
    guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", guiLanguage);
    useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);
    if (!sys().isRegularDS()) {
        useGbarunner = true;
    }

	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", soundfreq);
    showlogo = settingsini.GetInt("SRLOADER", "SHOWLOGO", showlogo);

	secondaryAccess = settingsini.GetInt("SRLOADER", "SECONDARY_ACCESS", secondaryAccess);
	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
    secondaryDevice = settingsini.GetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
    theme = settingsini.GetInt("SRLOADER", "THEME", theme);
    subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", subtheme);
    showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
    showBoxArt = settingsini.GetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
    animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);
	if (consoleModel < 2) {
		sysRegion = settingsini.GetInt("SRLOADER", "SYS_REGION", sysRegion);
		launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}

    flashcard = settingsini.GetInt("SRLOADER", "FLASHCARD", flashcard);

    slot1LaunchMethod = settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
    bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);

    // Default nds-bootstrap settings
    bstrap_language = settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", bstrap_language);
    boostCpu = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
    boostVram = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
    soundFix = settingsini.GetInt("NDS-BOOTSTRAP", "SOUND_FIX", soundFix);
    bstrap_dsiMode = settingsini.GetInt("NDS-BOOTSTRAP", "DSI_MODE", bstrap_dsiMode);

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

    show12hrClock =  settingsini.GetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);

    autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", autorun);

}

void DSiMenuPlusPlusSettings::saveSettings()
{
    CIniFile settingsini(DSIMENUPP_INI);

    settingsini.SetString("SRLOADER", "ROM_FOLDER", romfolder);

    settingsini.SetInt("SRLOADER", "PAGE_NUMBER", pagenum);
    settingsini.SetInt("SRLOADER", "CURSOR_POSITION", cursorPosition);
    settingsini.SetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", startMenu_cursorPosition);
    settingsini.SetInt("SRLOADER", "CONSOLE_MODEL", consoleModel);
    settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
    // Customizable UI settings.
    settingsini.SetInt("SRLOADER", "LANGUAGE", guiLanguage);
    settingsini.SetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);

	settingsini.SetInt("SRLOADER", "SOUND_FREQ", soundfreq);
    settingsini.SetInt("SRLOADER", "SHOWLOGO", showlogo);

    settingsini.SetInt("SRLOADER", "SECONDARY_ACCESS", secondaryAccess);
    settingsini.SetInt("SRLOADER", "THEME", theme);
    settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
    settingsini.SetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
    settingsini.SetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
    settingsini.SetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);
	if (consoleModel < 2) {
		settingsini.SetInt("SRLOADER", "SYS_REGION", sysRegion);
		settingsini.SetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}

    settingsini.SetInt("SRLOADER", "FLASHCARD", flashcard);

    settingsini.SetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
    settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);

    // Default nds-bootstrap settings
    settingsini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", bstrap_language);
    settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
    settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
    settingsini.SetInt("NDS-BOOTSTRAP", "SOUND_FIX", soundFix);
    settingsini.SetInt("NDS-BOOTSTRAP", "DSI_MODE", bstrap_dsiMode);

    settingsini.SetInt("SRLOADER", "AK_VIEWMODE", ak_viewMode);
    settingsini.SetInt("SRLOADER", "AK_SCROLLSPEED", ak_scrollSpeed);
    settingsini.SetString("SRLOADER", "AK_THEME", ak_theme);
    settingsini.SetInt("SRLOADER", "AK_ZOOM_ICONS", ak_zoomIcons);

    settingsini.SetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);
    settingsini.SetInt("SRLOADER", "APP_NAME", appName);

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

const char* DSiMenuPlusPlusSettings::getAppName()
{
    switch(appName)
    {
        case ENameTWiLightMenu:
        default:
            return "TWiLight Menu++";
        case ENameSRLoader:
            return "SRLoader";
        case ENameDSiMenuPP:
            return "DSiMenu++";
    }
}