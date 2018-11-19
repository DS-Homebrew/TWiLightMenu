// Language functions.
#ifndef DSIMENUPP_LANGUAGE_H
#define DSIMENUPP_LANGUAGE_H

// Strings
extern std::string STR_SAVING_SETTINGS;
extern std::string STR_SETTINGS_SAVED;

extern std::string STR_LR_SWITCH;
extern std::string STR_GUI_SETTINGS;
extern std::string STR_GAMESAPPS_SETTINGS;

// GUI settings
extern std::string STR_S1SDACCESS;
extern std::string STR_THEME;
extern std::string STR_LASTPLAYEDROM;
extern std::string STR_DSIMENUPPLOGO;
extern std::string STR_DIRECTORIES;
extern std::string STR_BOXART;
extern std::string STR_ANIMATEDSIICONS;
extern std::string STR_SYSREGION;
extern std::string STR_LAUNCHERAPP;
extern std::string STR_SYSTEMSETTINGS;
extern std::string STR_REPLACEDSIMENU;
extern std::string STR_RESTOREDSIMENU;

extern std::string STR_SHOW;
extern std::string STR_HIDE;

extern std::string STR_DESCRIPTION_S1SDACCESS_1;

extern std::string STR_DESCRIPTION_THEME_1;

extern std::string STR_DESCRIPTION_LASTPLAYEDROM_1;

extern std::string STR_DESCRIPTION_DSIMENUPPLOGO_1;

extern std::string STR_DESCRIPTION_DIRECTORIES_1;

extern std::string STR_DESCRIPTION_BOXART_1;

extern std::string STR_DESCRIPTION_ANIMATEDSIICONS_1;

extern std::string STR_DESCRIPTION_SYSREGION_1;

extern std::string STR_DESCRIPTION_LAUNCHERAPP_1;

extern std::string STR_DESCRIPTION_SYSTEMSETTINGS_1;

extern std::string STR_DESCRIPTION_REPLACEDSIMENU_1;

extern std::string STR_DESCRIPTION_RESTOREDSIMENU_1;

// Games/Apps settings
extern std::string STR_LANGUAGE;
extern std::string STR_CPUSPEED;
extern std::string STR_VRAMBOOST;
extern std::string STR_SOUNDFIX;
extern std::string STR_DEBUG;
extern std::string STR_LOGGING;
extern std::string STR_ROMREADLED;
extern std::string STR_RUNIN;
extern std::string STR_SNDFREQ;
extern std::string STR_SLOT1LAUNCHMETHOD;
extern std::string STR_LOADINGSCREEN;
extern std::string STR_BOOTSTRAP;
extern std::string STR_USEGBARUNNER2;

extern std::string STR_SYSTEM;
extern std::string STR_ON;
extern std::string STR_OFF;
extern std::string STR_YES;
extern std::string STR_NO;
extern std::string STR_NONE;
extern std::string STR_POWER;
extern std::string STR_CAMERA;
extern std::string STR_REBOOT;
extern std::string STR_DIRECT;
extern std::string STR_REGULAR;
extern std::string STR_RELEASE;
extern std::string STR_NIGHTLY;

extern std::string STR_DESCRIPTION_LANGUAGE_1;

extern std::string STR_DESCRIPTION_RUNIN_1;

extern std::string STR_DESCRIPTION_CPUSPEED_1;

extern std::string STR_DESCRIPTION_VRAMBOOST_1;

extern std::string STR_DESCRIPTION_SOUNDFIX_1;

extern std::string STR_DESCRIPTION_DEBUG_1;

extern std::string STR_DESCRIPTION_LOGGING_1;

extern std::string STR_DESCRIPTION_ROMREADLED_1;

extern std::string STR_DESCRIPTION_SNDFREQ_1;

extern std::string STR_DESCRIPTION_SLOT1LAUNCHMETHOD_1;

extern std::string STR_DESCRIPTION_LOADINGSCREEN_1;

extern std::string STR_DESCRIPTION_BOOTSTRAP_1;

extern std::string STR_DESCRIPTION_FLASHCARD_1;

extern std::string STR_DESCRIPTION_GBARUNNER2_1;

// Flashcard settings
extern std::string STR_FLASHCARD_SELECT;
extern std::string STR_LEFTRIGHT_FLASHCARD;
extern std::string STR_AB_SETRETURN;

// Sub-theme select
extern std::string STR_SUBTHEMESEL_DSI;

extern std::string STR_SUBTHEMESEL_R4;
extern std::string STR_SUBTHEMESEL_AK;

extern std::string STR_AB_SETSUBTHEME;
extern std::string STR_DSI_DARKMENU;
extern std::string STR_DSI_NORMALMENU;
extern std::string STR_DSI_RED;
extern std::string STR_DSI_BLUE;
extern std::string STR_DSI_GREEN;
extern std::string STR_DSI_YELLOW;
extern std::string STR_DSI_PINK;
extern std::string STR_DSI_PURPLE;
extern std::string STR_R4_THEME01;
extern std::string STR_R4_THEME02;
extern std::string STR_R4_THEME03;
extern std::string STR_R4_THEME04;
extern std::string STR_R4_THEME05;
extern std::string STR_R4_THEME06;
extern std::string STR_R4_THEME07;
extern std::string STR_R4_THEME08;
extern std::string STR_R4_THEME09;
extern std::string STR_R4_THEME10;
extern std::string STR_R4_THEME11;
extern std::string STR_R4_THEME12;
extern std::string STR_R4_THEME13;
extern std::string STR_R4_THEME14;

extern std::string STR_DEFAULT_LAUNCHER;
extern std::string STR_DESCRIPTION_DEFAULT_LAUNCHER_1;

extern std::string STR_12_HOUR_CLOCK;
extern std::string STR_DESCRIPTION_12_HOUR_CLOCK;

extern std::string STR_AK_ZOOMING_ICON;
extern std::string STR_DESCRIPTION_AK_ZOOMING_ICON;

extern std::string STR_AK_SCROLLSPEED;
extern std::string STR_DESCRIPTION_AK_SCROLLSPEED;


/**
 * Initialize translations.
 * Uses the language ID specified in settings.ui.language.
 *
 * Check the language variable outside of settings to determine
 * the actual language in use.
 */
void langInit(void);

#endif /* DSIMENUPP_LANGUAGE_H */
