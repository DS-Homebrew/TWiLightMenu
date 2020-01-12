
#include "dsimenusettings.h"
#include "bootstrappaths.h"
#include "systemdetails.h"
#include "common/inifile.h"
#include "common/flashcard.h"
#include <string.h>

static const char* settingsinipath = DSIMENUPP_INI;

DSiMenuPlusPlusSettings::DSiMenuPlusPlusSettings()
{
	romfolder[0] = "sd:/";
	romfolder[1] = "fat:/";
	pagenum = 0;
	cursorPosition = 0;
	startMenu_cursorPosition = 0;
	consoleModel = 0;

	gotosettings = false;
	guiLanguage = ELangDefault;
	useGbarunner = false;
	gbar2DldiAccess = false;
	theme = 0;
	subtheme = 0;

	showNds = true;
	showRvid = true;
	showNes = true;
	showGb = true;
	showSmsGg = true;
	showMd = true;
	showSnes = true;
	showDirectories = true;
	showBoxArt = true;
	animateDsiIcons = true;
	launcherApp = -1;
	previousUsedDevice = false;
	secondaryDevice = false;
	fcSaveOnSd = false;

	slot1LaunchMethod = EReboot;

	useBootstrap = isDSiMode();
	bootstrapFile = EReleaseBootstrap;

	bstrap_language = ELangDefault;
	boostCpu = false;
	boostVram = false;
	bstrap_dsiMode = EDSMode;
	forceSleepPatch = false;
	dsiWareBooter = false;

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
	showlogo = true;
	autorun = false;

	wideScreen = false;
}

void DSiMenuPlusPlusSettings::loadSettings()
{
	if (access(settingsinipath, F_OK) != 0 && flashcardFound()) {
		settingsinipath = DSIMENUPP_INI_FC;		// Fallback to .ini path on flashcard, if not found on SD card, or if SD access is disabled
	}

	CIniFile settingsini(settingsinipath);

	// UI settings.
	romfolder[0] = settingsini.GetString("SRLOADER", "ROM_FOLDER", romfolder[0]);
	romfolder[1] = settingsini.GetString("SRLOADER", "SECONDARY_ROM_FOLDER", romfolder[1]);

	pagenum = settingsini.GetInt("SRLOADER", "PAGE_NUMBER", pagenum);
	cursorPosition = settingsini.GetInt("SRLOADER", "CURSOR_POSITION", cursorPosition);
	startMenu_cursorPosition = settingsini.GetInt("SRLOADER", "STARTMENU_CURSOR_POSITION", startMenu_cursorPosition);
	consoleModel = settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", consoleModel);

	showNds = settingsini.GetInt("SRLOADER", "SHOW_NDS", showNds);
	showRvid = settingsini.GetInt("SRLOADER", "SHOW_RVID", showRvid);
	showNes = settingsini.GetInt("SRLOADER", "SHOW_NES", showNes);
	showGb = settingsini.GetInt("SRLOADER", "SHOW_GB", showGb);
	showSmsGg = settingsini.GetInt("SRLOADER", "SHOW_SMSGG", showSmsGg);
	showMd = settingsini.GetInt("SRLOADER", "SHOW_MDGEN", showMd);
	showSnes = settingsini.GetInt("SRLOADER", "SHOW_SNES", showSnes);

	// Customizable UI settings.
	guiLanguage = settingsini.GetInt("SRLOADER", "LANGUAGE", guiLanguage);
	useGbarunner = settingsini.GetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);
	if (!sys().isRegularDS()) {
		useGbarunner = true;
	}
	gbar2DldiAccess = settingsini.GetInt("SRLOADER", "GBAR2_DLDI_ACCESS", gbar2DldiAccess);

	soundfreq = settingsini.GetInt("SRLOADER", "SOUND_FREQ", soundfreq);
	showlogo = settingsini.GetInt("SRLOADER", "SHOWLOGO", showlogo);

	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
	secondaryDevice = bothSDandFlashcard() ? settingsini.GetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice) : flashcardFound();
	fcSaveOnSd = settingsini.GetInt("SRLOADER", "FC_SAVE_ON_SD", fcSaveOnSd);

	theme = settingsini.GetInt("SRLOADER", "THEME", theme);
	subtheme = settingsini.GetInt("SRLOADER", "SUB_THEME", subtheme);
	showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
	showBoxArt = settingsini.GetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
	animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);
	if (consoleModel < 2) {
		sysRegion = settingsini.GetInt("SRLOADER", "SYS_REGION", sysRegion);
		launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}

	slot1LaunchMethod = settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
	bootstrapFile = settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);
	useBootstrap = settingsini.GetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);

	// Default nds-bootstrap settings
	bstrap_language = settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", bstrap_language);
	boostCpu = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
	boostVram = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
	bstrap_dsiMode = settingsini.GetInt("NDS-BOOTSTRAP", "DSI_MODE", bstrap_dsiMode);
	forceSleepPatch = settingsini.GetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", forceSleepPatch);
	dsiWareBooter = settingsini.GetInt("SRLOADER", "DSIWARE_BOOTER", dsiWareBooter);

	ak_viewMode = settingsini.GetInt("SRLOADER", "AK_VIEWMODE", ak_viewMode);
	ak_scrollSpeed = settingsini.GetInt("SRLOADER", "AK_SCROLLSPEED", ak_scrollSpeed);
	ak_theme = settingsini.GetString("SRLOADER", "AK_THEME", ak_theme);
	ak_zoomIcons = settingsini.GetInt("SRLOADER", "AK_ZOOM_ICONS", ak_zoomIcons);

	dsiWareSrlPath = settingsini.GetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
	dsiWarePubPath = settingsini.GetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
	dsiWarePrvPath = settingsini.GetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
	launchType = settingsini.GetInt("SRLOADER", "LAUNCH_TYPE", launchType);
	romPath = settingsini.GetString("SRLOADER", "ROM_PATH", romPath);
	homebrewArg = settingsini.GetString("SRLOADER", "HOMEBREW_ARG", homebrewArg);
	homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);

	show12hrClock =  settingsini.GetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);
	snesEmulator =  settingsini.GetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);

	autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", autorun);
	wideScreen = settingsini.GetInt("SRLOADER", "WIDESCREEN", wideScreen);
}

void DSiMenuPlusPlusSettings::saveSettings()
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
	settingsini.SetInt("SRLOADER", "USE_GBARUNNER2", useGbarunner);

	if (bothSDandFlashcard()) {
		settingsini.SetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
	}
	settingsini.SetInt("SRLOADER", "THEME", theme);
	settingsini.SetInt("SRLOADER", "SUB_THEME", subtheme);
	settingsini.SetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
	settingsini.SetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
	settingsini.SetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);

	settingsini.SetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);
	settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);
	if (!isDSiMode()) settingsini.SetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);

	// Default nds-bootstrap settings
	/*settingsini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", bstrap_language);
	settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU", boostCpu);
	settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_VRAM", boostVram);
	settingsini.SetInt("NDS-BOOTSTRAP", "ASYNC_PREFETCH", bstrap_asyncPrefetch);*/

	settingsini.SetInt("SRLOADER", "AK_VIEWMODE", ak_viewMode);
	settingsini.SetInt("SRLOADER", "AK_SCROLLSPEED", ak_scrollSpeed);
	settingsini.SetString("SRLOADER", "AK_THEME", ak_theme);
	settingsini.SetInt("SRLOADER", "AK_ZOOM_ICONS", ak_zoomIcons);

	if (!gotosettings) {
		settingsini.SetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
		settingsini.SetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
		settingsini.SetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
		settingsini.SetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
		settingsini.SetInt("SRLOADER", "LAUNCH_TYPE", launchType);
		settingsini.SetString("SRLOADER", "ROM_PATH", romPath);
		settingsini.SetString("SRLOADER", "HOMEBREW_ARG", homebrewArg);
		settingsini.SetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);
	}

	settingsini.SetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);
	settingsini.SetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);

	settingsini.SaveIniFileModified();
}

u32 DSiMenuPlusPlusSettings::CopyBufferSize(void)
{
	return 0x8000;
}

DSiMenuPlusPlusSettings::TLanguage DSiMenuPlusPlusSettings::getGuiLanguage()
{
	return (TLanguage)(guiLanguage == ELangDefault ? PersonalData->language : guiLanguage);
}
