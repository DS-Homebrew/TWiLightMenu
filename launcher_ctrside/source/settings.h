#ifndef DSIMENUPP_SETTINGS_H
#define DSIMENUPP_SETTINGS_H

#include <string>
#include "graphic.h"
#include "pp2d/pp2d.h"

/** Settings **/

// Color settings.
// Use SET_ALPHA() to replace the alpha value.
#define SET_ALPHA(color, alpha) ((((alpha) & 0xFF) << 24) | ((color) & 0x00FFFFFF))

// 3D offsets.
typedef struct _Offset3D {
	float logo;
	float launchertext;
} Offset3D;
extern Offset3D offset3D[2];	// 0 == Left; 1 == Right

typedef struct _Settings_t {
	struct {
		bool toplayout;
		//	0: Show box art
		//	1: Hide box art
	} romselect;

	// TODO: Use int8_t instead of int?
	struct {
		int showbootscreen;	// 0 = No, 1 = Before ROM select screen, 2 = After launching a ROM
		int bootscreen;	// 0 = Nintendo DS, 1 = Nintendo DS (OAR), 2 = Nintendo DSi
		bool healthsafety;
	} ui;

	struct {
		int consoleModel;
		int rainbowled;	// 0 = None, 1 = Green, 2 = Rainbow
	} twl;
} Settings_t;
extern Settings_t settings;

/**
 * Load settings.
 */
void LoadSettings(void);

/**
 * Save settings.
 */
void SaveSettings(void);

#endif /* DSIMENUPP_SETTINGS_H */
