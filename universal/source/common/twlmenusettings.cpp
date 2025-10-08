#include "common/bootstrappaths.h"
#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/inifile.h"
#include "common/systemdetails.h"
#include "myDSiMode.h"

#include <nds/arm9/dldi.h>
#include <string.h>

const char *charUnlaunchBg;
int *removeLauncherPatchesPtr;

TWLSettings::TWLSettings()
{
	romfolder[0] = "sd:/";
	romfolder[1] = "fat:/";

	defaultRomfolder[0] = "null";
	defaultRomfolder[1] = "null";

	cursorAlwaysAtStart = false;

	pagenum[0] = 0;
	pagenum[1] = 0;

	cursorPosition[0] = 0;
	cursorPosition[1] = 0;

	// -1 means it won't overwrite cursorPosition / pagenum for saving
	saveCursorPosition[0] = -1;
	saveCursorPosition[1] = -1;

	consoleModel = EDSiRetail;
	languageSet = false;
	regionSet = false;

	logging = false;
	guiLanguage = ELangDefault;
	currentLanguage = ELangDefault;
	titleLanguage = ELangDefault;
	macroMode = false;
	// blfLevel = 0;
	sleepMode = true;
	kioskMode = false;
	dsiWareExploit = EExploitNone;
	wifiLed = true;
	wifiLedVer = 0;
	powerLedColor = false;
	sdRemoveDetect = true;
	showMicroSd = false;
	gbar2DldiAccess = false;
	showMainMenu = false;
	showSelectMenu = false;
	rocketRobzLogo = true;
	theme = EThemeDSi;
	settingsMusic = ESMusicTheme;
	dsiMusic = EMusicTheme;
	boxArtColorDeband = false;

	gbaBooter = isDSiMode() ? EGbaGbar2 : EGbaNativeGbar2;
	gbaR3Test = false;
	colEmulator = EColSegaColecoDS;
	sgEmulator = EColSegaColecoDS;
	mdEmulator = EMegaDriveHybrid;
	//snesEmulator = true;
	updateRecentlyPlayedList = true;
	sortMethod = ESortAlphabetical;
	hideEmptyBoxes = false;
	showDirectories = true;
	showHidden = false;
	showPhoto = true;
	showBoxArt = 1;
	filenameDisplay = 0;
	animateDsiIcons = true;
	showCustomIcons = true;
	preventDeletion = false;
	sysRegion = ERegionDefault;
	launcherApp = -1;
	secondaryAccess = false;
	previousUsedDevice = !sys().isRunFromSD();
	secondaryDevice = !sys().isRunFromSD();
	saveLocation = ESavesFolder;

	slot1LaunchMethod = EDirect;

	dsiSplash = isDSiMode();
	oppositeSplash = false;
	dsiSplashEasterEggs = true;
	dsiSplashAutoSkip = false;
	nintendoLogoColor = 1;
	showlogo = true;
	longSplashJingle = false;
	autorun = false;
	autostartSlot1 = false;

	show12hrClock = true;

	r4_theme = "unused";
	dsi_theme = "dark";
	_3ds_theme = "light";

	gbaBorder = "default.png";
	unlaunchBg = "default.gif";
	removeLauncherPatches = 2; // 2 == 'Default', keep splash/sound but allow the rest of the patches
	font = "default";
	useThemeFont = true;
	dsClassicCustomFont = false;

	dontShowClusterWarning = false;
	ignoreBlacklists = false;

	slot1AccessSD = false;
	slot1SCFGUnlock = false;
	ezFlashRam = false;
	limitedMode = 0;
	dontShowDSiWareInDSModeWarning = false;
	dsiWareBooter = EDSiWareBootstrap;
	dsiWareToSD = true;
	newSnesEmuVer = false;
	smsGgInRam = false;
	esrbRatingScreen = false;

	ak_viewMode = EViewInternal;
	// ak_scrollSpeed = EScrollFast;
	ak_theme = "zelda";
	ak_zoomIcons = true;

	useBootstrap = true;
	btsrpBootloaderDirect = false;
	bootstrapFile = EReleaseBootstrap;
	kernelUseable = true;

	internetBrowserLaunched = false;
	slot1Launched = false;
	launchType[0] = ENoLaunch;
	launchType[1] = ENoLaunch;
	homebrewBootstrap = EReleaseBootstrap;
	homebrewHasWide = false;

	//screenScaleSize = 0;
	wideScreen = false;

	gameLanguage = ELangDefault;
	gameRegion = ERegionDefault;
	useRomRegion = true;
	boostCpuForClut = true;
	forceSleepPatch = false;
	soundFreq = EFreq32KHz;
	saveRelocation = ERelocOnSDCard;
}

void TWLSettings::loadSettings()
{
	if (!sys().isRunFromSD()) {
		settingsinipath = DSIMENUPP_INI_FC; // Fallback to .ini path on flashcard, if TWLMenu++ is not found on SD card
	}

	CIniFile settingsini(settingsinipath);

	// UI settings.
	romfolder[0] = settingsini.GetString("SRLOADER", "ROM_FOLDER", romfolder[0]);
	defaultRomfolder[0] = settingsini.GetString("SRLOADER", "INITIAL_ROM_FOLDER", "null");

	romfolder[1] = settingsini.GetString("SRLOADER", "SECONDARY_ROM_FOLDER", romfolder[1]);
	defaultRomfolder[1] = settingsini.GetString("SRLOADER", "INITIAL_SECONDARY_ROM_FOLDER", "null");

	// Overwrite rom folder with the default one, if available.
	bool usingdefaultdir[2] = { false, false };
	for (int i = 0; i < 2; ++i) { 
		if (!defaultRomfolder[i].empty() && defaultRomfolder[i] != "null") {
			romfolder[i] = defaultRomfolder[i];
			usingdefaultdir[i] = true;
		}
	}

	if (sdFound() && (strncmp(romfolder[0].c_str(), "sd:", 3) != 0 || access(romfolder[0].c_str(), F_OK) != 0)) {
		romfolder[0] = "sd:/";
		usingdefaultdir[0] = true;
	}
	if (flashcardFound() && (strncmp(romfolder[1].c_str(), "fat:", 4) != 0 || access(romfolder[1].c_str(), F_OK) != 0)) {
		romfolder[1] = "fat:/";
		usingdefaultdir[1] = true;
	}

	romPath[0] = settingsini.GetString("SRLOADER", "ROM_PATH", romPath[0]);
	romPath[1] = settingsini.GetString("SRLOADER", "SECONDARY_ROM_PATH", romPath[1]);
	if (strncmp(romPath[0].c_str(), "sd:", 3) != 0) {
		romPath[0] = "";
	}
	if (strncmp(romPath[1].c_str(), "fat:", 4) != 0) {
		romPath[1] = "";
	}

	cursorAlwaysAtStart = settingsini.GetInt("SRLOADER", "CURSOR_ALWAYS_AT_START", cursorAlwaysAtStart);
	if (!cursorAlwaysAtStart) {
		// Only remember cursor pos if we don't have default dirs
		if (!usingdefaultdir[0]) {
			pagenum[0] = settingsini.GetInt("SRLOADER", "PAGE_NUMBER", pagenum[0]);
			cursorPosition[0] = settingsini.GetInt("SRLOADER", "CURSOR_POSITION", cursorPosition[0]);
		}
		if (!usingdefaultdir[1]) {
			pagenum[1] = settingsini.GetInt("SRLOADER", "SECONDARY_PAGE_NUMBER", pagenum[1]);
			cursorPosition[1] = settingsini.GetInt("SRLOADER", "SECONDARY_CURSOR_POSITION", cursorPosition[1]);
		}
	}

	consoleModel = (TConsoleModel)settingsini.GetInt("SRLOADER", "CONSOLE_MODEL", consoleModel);
	languageSet = settingsini.GetInt("SRLOADER", "LANGUAGE_SET", languageSet);
	regionSet = settingsini.GetInt("SRLOADER", "REGION_SET", regionSet);

	// Customizable UI settings.
	logging = settingsini.GetInt("SRLOADER", "LOGGING", logging);
	guiLanguage = (TLanguage)settingsini.GetInt("SRLOADER", "LANGUAGE", guiLanguage);
	currentLanguage = guiLanguage;
	titleLanguage = (TLanguage)settingsini.GetInt("SRLOADER", "TITLELANGUAGE", titleLanguage);
	macroMode = settingsini.GetInt("SRLOADER", "MACRO_MODE", macroMode);
	sleepMode = settingsini.GetInt("SRLOADER", "SLEEP_MODE", sleepMode);
	kioskMode = settingsini.GetInt("SRLOADER", "KIOSK_MODE", kioskMode);
	dsiWareExploit = (TExploit)settingsini.GetInt("SRLOADER", "DSIWARE_EXPLOIT", dsiWareExploit);
	wifiLed = settingsini.GetInt("SRLOADER", "WIFI_LED", wifiLed);
	wifiLedVer = settingsini.GetInt("SRLOADER", "WIFI_LED_VER", wifiLedVer);
	if (wifiLedVer == 0) {
		wifiLed = true; // Set to enable by default
		wifiLedVer = 1;
	}
	powerLedColor = settingsini.GetInt("SRLOADER", "POWER_LED_COLOR", powerLedColor);
	sdRemoveDetect = settingsini.GetInt("SRLOADER", "SD_REMOVE_DETECT", sdRemoveDetect);
	showMicroSd = settingsini.GetInt("SRLOADER", "SHOW_MICROSD", showMicroSd);
	gbar2DldiAccess = settingsini.GetInt("SRLOADER", "GBAR2_DLDI_ACCESS", gbar2DldiAccess);
	showMainMenu = settingsini.GetInt("SRLOADER", "SHOW_MAIN_MENU", showMainMenu);
	showSelectMenu = settingsini.GetInt("SRLOADER", "SHOW_SELECT_MENU", showSelectMenu);
	rocketRobzLogo = settingsini.GetInt("SRLOADER", "ROCKET_ROBZ_LOGO", rocketRobzLogo);
	theme = (TTheme)settingsini.GetInt("SRLOADER", "THEME", theme);
	settingsMusic = (TSettingsMusic)settingsini.GetInt("SRLOADER", "SETTINGS_MUSIC", settingsMusic);
	dsiMusic = (TDSiMusic)settingsini.GetInt("SRLOADER", "DSI_MUSIC", dsiMusic);
	boxArtColorDeband = settingsini.GetInt("SRLOADER", "PHOTO_BOXART_COLOR_DEBAND", boxArtColorDeband);

	if (sys().isRegularDS()) {
		gbaBooter = (TGbaBooter)settingsini.GetInt("SRLOADER", "SHOW_GBA", gbaBooter);
		if (gbaBooter == 0) // 0 (don't show) is deprecated
			gbaBooter = EGbaNativeGbar2;
	} else {
		gbaBooter = EGbaGbar2;
	}
	// gbaR3Test = settingsini.GetInt("SRLOADER", "GBARUNNER3_TEST", gbaR3Test);
	colEmulator = (TColSegaEmulator)settingsini.GetInt("SRLOADER", "SHOW_COL", colEmulator);
	if (colEmulator == 0) // 0 (don't show) is deprecated
		colEmulator = EColSegaColecoDS;
	sgEmulator = (TColSegaEmulator)settingsini.GetInt("SRLOADER", "SHOW_SG", sgEmulator);
	if (sgEmulator == 0) // 0 (don't show) is deprecated
		sgEmulator = EColSegaColecoDS;
	if (!(isDSiMode() && (access("sd:/", F_OK) == 0) && sys().arm7SCFGLocked())) {
		mdEmulator = (TMegaDriveEmulator)settingsini.GetInt("SRLOADER", "SHOW_MDGEN", mdEmulator);
		if (mdEmulator == 0) // 0 (don't show) is deprecated
			mdEmulator = EMegaDriveHybrid;
	} else {
		mdEmulator = EMegaDrivePico; // Use only PicoDriveTWL
	}
	//snesEmulator = settingsini.GetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);
	updateRecentlyPlayedList = settingsini.GetInt("SRLOADER", "UPDATE_RECENTLY_PLAYED_LIST", updateRecentlyPlayedList);
	sortMethod = (TSortMethod)settingsini.GetInt("SRLOADER", "SORT_METHOD", sortMethod);
	hideEmptyBoxes = settingsini.GetInt("SRLOADER", "HIDE_EMPTY_BOXES", hideEmptyBoxes);
	showDirectories = settingsini.GetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
	showHidden = settingsini.GetInt("SRLOADER", "SHOW_HIDDEN", showHidden);
	showPhoto = settingsini.GetInt("SRLOADER", "SHOW_PHOTO", showPhoto);
	showBoxArt = settingsini.GetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
	if (!dsiFeatures() && showBoxArt == 2) // Reset to 1 if not in DSi mode
		showBoxArt = 1;
	filenameDisplay = settingsini.GetInt("SRLOADER", "FILENAME_DISPLAY", filenameDisplay);
	animateDsiIcons = settingsini.GetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);
	showCustomIcons = settingsini.GetInt("SRLOADER", "SHOW_CUSTOM_ICONS", showCustomIcons);
	preventDeletion = settingsini.GetInt("SRLOADER", "PREVENT_ROM_DELETION", preventDeletion);
	sysRegion = (TRegion)settingsini.GetInt("SRLOADER", "SYS_REGION", sysRegion);
	if (consoleModel < 2) {
		launcherApp = settingsini.GetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}
	secondaryAccess = settingsini.GetInt("SRLOADER", "SECONDARY_ACCESS", secondaryAccess);
	previousUsedDevice = settingsini.GetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
	secondaryDevice = bothSDandFlashcard() ? settingsini.GetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice) : flashcardFound();
	saveLocation = (TSaveLoc)settingsini.GetInt("SRLOADER", "SAVE_LOCATION", saveLocation);
	settingsini.GetStringVector("SRLOADER", "BLOCKED_EXTENSIONS", blockedExtensions, ':');

	slot1LaunchMethod = (TSlot1LaunchMethod)settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);

	dsiSplash = settingsini.GetInt("SRLOADER", "DSI_SPLASH", dsiSplash);
	oppositeSplash = settingsini.GetInt("SRLOADER", "OPPOSITE_SPLASH", oppositeSplash);
	dsiSplashEasterEggs = settingsini.GetInt("SRLOADER", "DSI_SPLASH_EASTER_EGGS", dsiSplashEasterEggs);
	dsiSplashAutoSkip = settingsini.GetInt("SRLOADER", "DSI_SPLASH_AUTO_SKIP", dsiSplashAutoSkip);
	nintendoLogoColor = settingsini.GetInt("SRLOADER", "NINTENDO_LOGO_COLOR", nintendoLogoColor);
	showlogo = settingsini.GetInt("SRLOADER", "SHOWLOGO", showlogo);
	longSplashJingle = settingsini.GetInt("SRLOADER", "LONG_SPLASH_JINGLE", longSplashJingle);
	autorun = settingsini.GetInt("SRLOADER", "AUTORUNGAME", autorun);
	autostartSlot1 = settingsini.GetInt("SRLOADER", "AUTORUNSLOT1", autostartSlot1);

	show12hrClock = settingsini.GetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);

	r4_theme = settingsini.GetString("SRLOADER", "R4_THEME", r4_theme);
	dsi_theme = settingsini.GetString("SRLOADER", "DSI_THEME", dsi_theme);
	_3ds_theme = settingsini.GetString("SRLOADER", "3DS_THEME", _3ds_theme);

	gbaBorder = settingsini.GetString("SRLOADER", "GBA_BORDER", gbaBorder);
	unlaunchBg = settingsini.GetString("SRLOADER", "UNLAUNCH_BG", unlaunchBg);
	charUnlaunchBg = unlaunchBg.c_str();
	removeLauncherPatches = settingsini.GetInt("SRLOADER", "UNLAUNCH_LAUNCHER_PATCHES", removeLauncherPatches);
	removeLauncherPatchesPtr = &removeLauncherPatches;
	font = settingsini.GetString("SRLOADER", "FONT", font);
	useThemeFont = settingsini.GetInt("SRLOADER", "USE_THEME_FONT", useThemeFont);
	dsClassicCustomFont = settingsini.GetInt("SRLOADER", "DS_CLASSIC_CUSTOM_FONT", dsClassicCustomFont);

	dontShowClusterWarning = settingsini.GetInt("SRLOADER", "DONT_SHOW_CLUSTER_WARNING", dontShowClusterWarning);
	ignoreBlacklists = settingsini.GetInt("SRLOADER", "IGNORE_BLACKLISTS", ignoreBlacklists);

	slot1AccessSD = settingsini.GetInt("SRLOADER", "SLOT1_ENABLESD", slot1AccessSD);
	slot1SCFGUnlock = settingsini.GetInt("SRLOADER", "SLOT1_SCFG_UNLOCK", slot1SCFGUnlock);
	ezFlashRam = settingsini.GetInt("SRLOADER", "EZ_FLASH_RAM", ezFlashRam);
	limitedMode = settingsini.GetInt("SRLOADER", "LIMITED_MODE", limitedMode);
	dontShowDSiWareInDSModeWarning = settingsini.GetInt("SRLOADER", "DONT_SHOW_DSIWARE_IN_DS_MODE_WARNING", dontShowDSiWareInDSModeWarning);
	dsiWareBooter = (TDSiWareBooter)settingsini.GetInt("SRLOADER", "DSIWARE_BOOTER", dsiWareBooter);
	dsiWareToSD = settingsini.GetInt("SRLOADER", "DSIWARE_TO_SD", dsiWareToSD);
	newSnesEmuVer = settingsini.GetInt("SRLOADER", "NEW_SNES_EMU_VER", newSnesEmuVer);
	smsGgInRam = settingsini.GetInt("SRLOADER", "SMS_GG_IN_RAM", smsGgInRam);
	esrbRatingScreen = settingsini.GetInt("SRLOADER", "ESRB_RATING_SCREEN", esrbRatingScreen);

	ak_viewMode = settingsini.GetInt("SRLOADER", "AK_VIEWMODE", ak_viewMode);
	// ak_scrollSpeed = settingsini.GetInt("SRLOADER", "AK_SCROLLSPEED", ak_scrollSpeed);
	ak_theme = settingsini.GetString("SRLOADER", "AK_THEME", ak_theme);
	ak_zoomIcons = settingsini.GetInt("SRLOADER", "AK_ZOOM_ICONS", ak_zoomIcons);

	kernelUseable = (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS);
	if (kernelUseable) {
		const bool woodKernel = (
		(memcmp(io_dldi_data->friendlyName, "R4iDSN", 6) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "R4iTT", 5) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "Acekard AK2", 11) == 0)
	 || (memcmp(io_dldi_data->friendlyName, "Ace3DS+", 7) == 0)
	);

		if (woodKernel) {
			kernelUseable = !dsiFeatures();
		}
	}

	if (kernelUseable) {
		useBootstrap = settingsini.GetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);
	} else {
		useBootstrap = true;
	}
	btsrpBootloaderDirect = settingsini.GetInt("SRLOADER", "BOOTSTRAP_BOOTLOADER_DIRECT", btsrpBootloaderDirect);
	bootstrapFile = (TBootstrapFile)settingsini.GetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);

	internetBrowserPath = settingsini.GetString("SRLOADER", "INTERNET_BROWSER_PATH", internetBrowserPath);
	internetBrowserLaunched = settingsini.GetInt("SRLOADER", "INTERNET_BROWSER_LAUNCHED", internetBrowserLaunched);
	dsiWareSrlPath = settingsini.GetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
	dsiWarePubPath = settingsini.GetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
	dsiWarePrvPath = settingsini.GetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
	slot1Launched = settingsini.GetInt("SRLOADER", "SLOT1_LAUNCHED", slot1Launched);
	launchType[0] = (TLaunchType)settingsini.GetInt("SRLOADER", "LAUNCH_TYPE", launchType[0]);
	launchType[1] = (TLaunchType)settingsini.GetInt("SRLOADER", "SECONDARY_LAUNCH_TYPE", launchType[1]);
	homebrewArg[0] = settingsini.GetString("SRLOADER", "HOMEBREW_ARG", homebrewArg[0]);
	homebrewArg[1] = settingsini.GetString("SRLOADER", "SECONDARY_HOMEBREW_ARG", homebrewArg[1]);
	homebrewBootstrap = settingsini.GetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);
	homebrewHasWide = settingsini.GetInt("SRLOADER", "HOMEBREW_HAS_WIDE", 0);

	//screenScaleSize = settingsini.GetInt("TWL_FIRM", "SCREENSCALESIZE", screenScaleSize);
	wideScreen = settingsini.GetInt("SRLOADER", "WIDESCREEN", wideScreen);

	// Default nds-bootstrap settings
	gameLanguage = (TLanguage)settingsini.GetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);
	gameRegion = (TRegion)settingsini.GetInt("NDS-BOOTSTRAP", "REGION", gameRegion);
	useRomRegion = settingsini.GetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", useRomRegion);
	boostCpuForClut = settingsini.GetInt("NDS-BOOTSTRAP", "BOOST_CPU_FOR_CLUT", boostCpuForClut);
	forceSleepPatch = settingsini.GetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", forceSleepPatch);
	soundFreq = (TSoundFreq)settingsini.GetInt("NDS-BOOTSTRAP", "SOUND_FREQ", soundFreq);
	saveRelocation = (TSaveRelocation)settingsini.GetInt("NDS-BOOTSTRAP", "SAVE_RELOCATION", saveRelocation);
}

void TWLSettings::saveSettings()
{
	CIniFile settingsini(settingsinipath);

	// UI settings.
	settingsini.SetString("SRLOADER", "ROM_FOLDER", romfolder[0]);
	settingsini.SetString("SRLOADER", "INITIAL_ROM_FOLDER", defaultRomfolder[0]);
	settingsini.SetString("SRLOADER", "SECONDARY_ROM_FOLDER", romfolder[1]);
	settingsini.SetString("SRLOADER", "INITIAL_SECONDARY_ROM_FOLDER", defaultRomfolder[1]);

	settingsini.SetString("SRLOADER", "ROM_PATH", romPath[0]);
	settingsini.SetString("SRLOADER", "SECONDARY_ROM_PATH", romPath[1]);

	settingsini.SetInt("SRLOADER", "CURSOR_ALWAYS_AT_START", cursorAlwaysAtStart);
	if (!cursorAlwaysAtStart) {
		// Cursor position.
		if (saveCursorPosition[0] == -1) {
			settingsini.SetInt("SRLOADER", "PAGE_NUMBER", pagenum[0]);
			settingsini.SetInt("SRLOADER", "CURSOR_POSITION", cursorPosition[0]);
		} else {
			// Overwrite cursor pos
			settingsini.SetInt("SRLOADER", "PAGE_NUMBER", saveCursorPosition[0]/40);
			settingsini.SetInt("SRLOADER", "CURSOR_POSITION", saveCursorPosition[0]%40);
		}

		if (saveCursorPosition[1] == -1) {
			settingsini.SetInt("SRLOADER", "SECONDARY_PAGE_NUMBER", pagenum[1]);
			settingsini.SetInt("SRLOADER", "SECONDARY_CURSOR_POSITION", cursorPosition[1]);
		} else {
			// Overwrite cursor pos
			settingsini.SetInt("SRLOADER", "SECONDARY_PAGE_NUMBER", saveCursorPosition[1]/40);
			settingsini.SetInt("SRLOADER", "SECONDARY_CURSOR_POSITION", saveCursorPosition[1]%40);
		}
	}

	settingsini.SetInt("SRLOADER", "CONSOLE_MODEL", consoleModel);
	settingsini.SetInt("SRLOADER", "LANGUAGE_SET", languageSet);
	settingsini.SetInt("SRLOADER", "REGION_SET", regionSet);

	// Customizable UI settings.
	settingsini.SetInt("SRLOADER", "LOGGING", logging);
	settingsini.SetInt("SRLOADER", "LANGUAGE", guiLanguage);
	settingsini.SetInt("SRLOADER", "TITLELANGUAGE", titleLanguage);
	settingsini.SetInt("SRLOADER", "MACRO_MODE", macroMode);
	settingsini.SetInt("SRLOADER", "SLEEP_MODE", sleepMode);
	settingsini.SetInt("SRLOADER", "DSIWARE_EXPLOIT", dsiWareExploit);
	settingsini.SetInt("SRLOADER", "WIFI_LED", wifiLed);
	settingsini.SetInt("SRLOADER", "WIFI_LED_VER", wifiLedVer);
	settingsini.SetInt("SRLOADER", "POWER_LED_COLOR", powerLedColor);
	settingsini.SetInt("SRLOADER", "SD_REMOVE_DETECT", sdRemoveDetect);
	settingsini.SetInt("SRLOADER", "SHOW_MICROSD", showMicroSd);
	settingsini.SetInt("SRLOADER", "GBAR2_DLDI_ACCESS", gbar2DldiAccess);
	settingsini.SetInt("SRLOADER", "SHOW_MAIN_MENU", showMainMenu);
	settingsini.SetInt("SRLOADER", "SHOW_SELECT_MENU", showSelectMenu);
	settingsini.SetInt("SRLOADER", "ROCKET_ROBZ_LOGO", rocketRobzLogo);
	settingsini.SetInt("SRLOADER", "THEME", theme);
	settingsini.SetInt("SRLOADER", "SETTINGS_MUSIC", settingsMusic);
	settingsini.SetInt("SRLOADER", "DSI_MUSIC", dsiMusic);
	settingsini.SetInt("SRLOADER", "PHOTO_BOXART_COLOR_DEBAND", boxArtColorDeband);

	if (sys().isRegularDS()) {
		settingsini.SetInt("SRLOADER", "SHOW_GBA", gbaBooter);
	}
	settingsini.SetInt("SRLOADER", "SHOW_COL", colEmulator);
	settingsini.SetInt("SRLOADER", "SHOW_SG", sgEmulator);
	if (!(isDSiMode() && (access("sd:/", F_OK) == 0) && sys().arm7SCFGLocked())) {
		settingsini.SetInt("SRLOADER", "SHOW_MDGEN", mdEmulator);
	}
	// settingsini.SetInt("SRLOADER", "SNES_EMULATOR", snesEmulator);
	settingsini.SetInt("SRLOADER", "UPDATE_RECENTLY_PLAYED_LIST", updateRecentlyPlayedList);
	settingsini.SetInt("SRLOADER", "SORT_METHOD", sortMethod);
	settingsini.SetInt("SRLOADER", "HIDE_EMPTY_BOXES", hideEmptyBoxes);
	settingsini.SetInt("SRLOADER", "SHOW_DIRECTORIES", showDirectories);
	settingsini.SetInt("SRLOADER", "SHOW_HIDDEN", showHidden);
	settingsini.SetInt("SRLOADER", "SHOW_PHOTO", showPhoto);
	settingsini.SetInt("SRLOADER", "SHOW_BOX_ART", showBoxArt);
	settingsini.SetInt("SRLOADER", "FILENAME_DISPLAY", filenameDisplay);
	settingsini.SetInt("SRLOADER", "ANIMATE_DSI_ICONS", animateDsiIcons);
	settingsini.SetInt("SRLOADER", "SHOW_CUSTOM_ICONS", showCustomIcons);
	settingsini.SetInt("SRLOADER", "PREVENT_ROM_DELETION", preventDeletion);
	settingsini.SetInt("SRLOADER", "SYS_REGION", sysRegion);
	if (consoleModel < 2) {
		settingsini.SetInt("SRLOADER", "LAUNCHER_APP", launcherApp);
	}
	settingsini.SetInt("SRLOADER", "SECONDARY_ACCESS", secondaryAccess);
	settingsini.SetInt("SRLOADER", "PREVIOUS_USED_DEVICE", previousUsedDevice);
	if (bothSDandFlashcard()) {
		settingsini.SetInt("SRLOADER", "SECONDARY_DEVICE", secondaryDevice);
	}
	settingsini.SetInt("SRLOADER", "SAVE_LOCATION", saveLocation);

	settingsini.SetInt("SRLOADER", "SLOT1_LAUNCHMETHOD", slot1LaunchMethod);

	settingsini.SetInt("SRLOADER", "DSI_SPLASH", dsiSplash);
	settingsini.SetInt("SRLOADER", "DSI_SPLASH_EASTER_EGGS", dsiSplashEasterEggs);
	settingsini.SetInt("SRLOADER", "DSI_SPLASH_AUTO_SKIP", dsiSplashAutoSkip);
	settingsini.SetInt("SRLOADER", "NINTENDO_LOGO_COLOR", nintendoLogoColor);
	settingsini.SetInt("SRLOADER", "SHOWLOGO", showlogo);
	settingsini.SetInt("SRLOADER", "LONG_SPLASH_JINGLE", longSplashJingle);
	settingsini.SetInt("SRLOADER", "AUTORUNGAME", autorun);
	settingsini.SetInt("SRLOADER", "AUTORUNSLOT1", autostartSlot1);

	settingsini.SetInt("SRLOADER", "SHOW_12H_CLOCK", show12hrClock);

	settingsini.SetString("SRLOADER", "R4_THEME", r4_theme);
	settingsini.SetString("SRLOADER", "DSI_THEME", dsi_theme);
	settingsini.SetString("SRLOADER", "3DS_THEME", _3ds_theme);

	settingsini.SetString("SRLOADER", "GBA_BORDER", gbaBorder);
	settingsini.SetString("SRLOADER", "UNLAUNCH_BG", unlaunchBg);
	settingsini.SetInt("SRLOADER", "UNLAUNCH_LAUNCHER_PATCHES", removeLauncherPatches);
	settingsini.SetString("SRLOADER", "FONT", font);
	settingsini.SetInt("SRLOADER", "USE_THEME_FONT", useThemeFont);
	settingsini.SetInt("SRLOADER", "DS_CLASSIC_CUSTOM_FONT", dsClassicCustomFont);

	settingsini.SetInt("SRLOADER", "DONT_SHOW_CLUSTER_WARNING", dontShowClusterWarning);
	settingsini.SetInt("SRLOADER", "IGNORE_BLACKLISTS", ignoreBlacklists);

	settingsini.SetInt("SRLOADER", "SLOT1_ENABLESD", slot1AccessSD);
	settingsini.SetInt("SRLOADER", "SLOT1_SCFG_UNLOCK", slot1SCFGUnlock);
	settingsini.SetInt("SRLOADER", "LIMITED_MODE", limitedMode);
	settingsini.SetInt("SRLOADER", "DONT_SHOW_DSIWARE_IN_DS_MODE_WARNING", dontShowDSiWareInDSModeWarning);
	settingsini.SetInt("SRLOADER", "DSIWARE_BOOTER", dsiWareBooter);
	settingsini.SetInt("SRLOADER", "DSIWARE_TO_SD", dsiWareToSD);
	settingsini.SetInt("SRLOADER", "SMS_GG_IN_RAM", smsGgInRam);
	settingsini.SetInt("SRLOADER", "ESRB_RATING_SCREEN", esrbRatingScreen);

	settingsini.SetInt("SRLOADER", "AK_VIEWMODE", ak_viewMode);
	// settingsini.SetInt("SRLOADER", "AK_SCROLLSPEED", ak_scrollSpeed);
	settingsini.SetString("SRLOADER", "AK_THEME", ak_theme);
	settingsini.SetInt("SRLOADER", "AK_ZOOM_ICONS", ak_zoomIcons);

	if (kernelUseable) {
		settingsini.SetInt("SRLOADER", "USE_BOOTSTRAP", useBootstrap);
	}
	settingsini.SetInt("SRLOADER", "BOOTSTRAP_BOOTLOADER_DIRECT", btsrpBootloaderDirect);
	settingsini.SetInt("SRLOADER", "BOOTSTRAP_FILE", bootstrapFile);

	settingsini.SetString("SRLOADER", "INTERNET_BROWSER_PATH", internetBrowserPath);
	settingsini.SetInt("SRLOADER", "INTERNET_BROWSER_LAUNCHED", internetBrowserLaunched);
	settingsini.SetString("SRLOADER", "DSIWARE_SRL", dsiWareSrlPath);
	settingsini.SetString("SRLOADER", "DSIWARE_PUB", dsiWarePubPath);
	settingsini.SetString("SRLOADER", "DSIWARE_PRV", dsiWarePrvPath);
	settingsini.SetInt("SRLOADER", "SLOT1_LAUNCHED", slot1Launched);
	settingsini.SetInt("SRLOADER", "LAUNCH_TYPE", launchType[0]);
	settingsini.SetInt("SRLOADER", "SECONDARY_LAUNCH_TYPE", launchType[1]);
	settingsini.SetString("SRLOADER", "HOMEBREW_ARG", homebrewArg[0]);
	settingsini.SetString("SRLOADER", "SECONDARY_HOMEBREW_ARG", homebrewArg[1]);
	settingsini.SetInt("SRLOADER", "HOMEBREW_BOOTSTRAP", homebrewBootstrap);
	settingsini.SetInt("SRLOADER", "HOMEBREW_HAS_WIDE", 0);

	// settingsini.SetInt("TWL_FIRM", "SCREENSCALESIZE", screenScaleSize);
	settingsini.SetInt("SRLOADER", "WIDESCREEN", wideScreen);

	// Default nds-bootstrap settings
	settingsini.SetInt("NDS-BOOTSTRAP", "LANGUAGE", gameLanguage);
	settingsini.SetInt("NDS-BOOTSTRAP", "REGION", gameRegion);
	settingsini.SetInt("NDS-BOOTSTRAP", "USE_ROM_REGION", useRomRegion);
	settingsini.SetInt("NDS-BOOTSTRAP", "BOOST_CPU_FOR_CLUT", boostCpuForClut);
	settingsini.SetInt("NDS-BOOTSTRAP", "FORCE_SLEEP_PATCH", forceSleepPatch);
	settingsini.SetInt("NDS-BOOTSTRAP", "SOUND_FREQ", soundFreq);
	settingsini.SetInt("NDS-BOOTSTRAP", "SAVE_RELOCATION", saveRelocation);

	settingsini.SaveIniFileModified(settingsinipath);
}

TWLSettings::TLanguage TWLSettings::getGuiLanguage()
{
	if (currentLanguage == ELangDefault) {
		extern bool useTwlCfg;
		return (TLanguage)(useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
	}
	return (TLanguage)currentLanguage;
}

TWLSettings::TLanguage TWLSettings::getGameLanguage()
{
	if (gameLanguage == ELangDefault) {
		extern bool useTwlCfg;
		return (TLanguage)(useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
	}
	return (TLanguage)gameLanguage;
}

TWLSettings::TLanguage TWLSettings::getTitleLanguage()
{
	if (titleLanguage == ELangDefault) {
		return (TLanguage)PersonalData->language;
	}
	return (TLanguage)titleLanguage;
}

std::string TWLSettings::getGuiLanguageString()
{
	switch (getGuiLanguage()) {
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
		case TWLSettings::ELangKazakh:
			return "kk";
		case TWLSettings::ELangGalician:
			return "gl";
	}
}

bool TWLSettings::rtl()
{
	switch (currentLanguage) {
		case ELangHebrew:
		case ELangArabic:
			return true;
		default:
			return false;
	}
}

TWLSettings::TRegion TWLSettings::getGameRegion()
{
	if (dsiFeatures() && gameRegion == ERegionDefault) {
		extern bool useTwlCfg;
		if (!useTwlCfg) {
			return ERegionUSA;
		}

		u8 twlCfgCountry = *(u8*)0x02000405;
		if (twlCfgCountry == 0x01) {
			return ERegionJapan;
		} else if (twlCfgCountry == 0xA0) {
			return ERegionChina;
		} else if (twlCfgCountry == 0x88) {
			return ERegionKorea;
		} else if (twlCfgCountry == 0x41 || twlCfgCountry == 0x5F) {
			return ERegionAustralia;
		} else if ((twlCfgCountry >= 0x08 && twlCfgCountry <= 0x34) || twlCfgCountry == 0x99 || twlCfgCountry == 0xA8) {
			return ERegionUSA;
		} else if (twlCfgCountry >= 0x40 && twlCfgCountry <= 0x70) {
			return ERegionEurope;
		}
		return ERegionUSA;
	}
	return (TRegion)gameRegion;
}
