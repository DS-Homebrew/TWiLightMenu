#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <3ds.h>
#include <malloc.h>
#include <sys/stat.h>

#include "graphic.h"
#include "pp2d/pp2d.h"
#include "settings.h"

#define CONFIG_3D_SLIDERSTATE (*(float *)0x1FF81080)

// 3D offsets. (0 == Left, 1 == Right)
Offset3D offset3D[2] = {{0.0f}, {0.0f}};

struct {
	int x;
	int y;
} buttons[] = {
	{ 17,  39},
	{169,  87},
};
const char *button_titles[] = {
	"Start DSiMenu++",
	"Start last-ran ROM",
};

void screenoff()
{
    gspLcdInit();\
    GSPLCD_PowerOffBacklight(GSPLCD_SCREEN_BOTH);\
    gspLcdExit();
}

void screenon()
{
    gspLcdInit();\
    GSPLCD_PowerOnBacklight(GSPLCD_SCREEN_BOTH);\
    gspLcdExit();
}

int main()
{
	aptInit();
	amInit();
	sdmcInit();
	romfsInit();
	srvInit();
	hidInit();

	pp2d_init();
	
	pp2d_set_screen_color(GFX_TOP, TRANSPARENT);
	pp2d_set_3D(1);
	
	Result res = 0;

	pp2d_load_texture_png(homeicontex, "romfs:/graphics/homeicon.png");
	pp2d_load_texture_png(topbgtex, "romfs:/graphics/top_bg.png");
	pp2d_load_texture_png(subbgtex, "romfs:/graphics/sub_bg.png");
	pp2d_load_texture_png(logotex, "romfs:/graphics/logo.png");
	pp2d_load_texture_png(buttontex, "romfs:/graphics/button.png");
	
	int menuSelection = 0;
	
	int fadealpha = 255;
	bool fadein = true;
	bool fadeout = false;
	
	// Loop as long as the status is not exit
	while(aptMainLoop()) {
		offset3D[0].logo = CONFIG_3D_SLIDERSTATE * -5.0f;
		offset3D[1].logo = CONFIG_3D_SLIDERSTATE * 5.0f;
		offset3D[0].launchertext = CONFIG_3D_SLIDERSTATE * -3.0f;
		offset3D[1].launchertext = CONFIG_3D_SLIDERSTATE * 3.0f;

		// Scan hid shared memory for input events
		hidScanInput();
		
		const u32 hDown = hidKeysDown();
		const u32 hHeld = hidKeysHeld();

		for (int topfb = GFX_LEFT; topfb <= GFX_RIGHT; topfb++) {
			if (topfb == GFX_LEFT) pp2d_begin_draw(GFX_TOP, (gfx3dSide_t)topfb);
			else pp2d_draw_on(GFX_TOP, (gfx3dSide_t)topfb);
			pp2d_draw_texture(topbgtex, 0, 0);
			pp2d_draw_texture(logotex, offset3D[topfb].logo+400/2 - 256/2, 240/2 - 128/2);
			pp2d_draw_text(offset3D[topfb].launchertext+224, 160, 1.00, 1.00, BLACK, "Launcher");
			if (fadealpha > 0) pp2d_draw_rectangle(0, 0, 400, 240, RGBA8(0, 0, 0, fadealpha)); // Fade in/out effect
		}
		pp2d_draw_on(GFX_BOTTOM, GFX_LEFT);
		pp2d_draw_texture(subbgtex, 0, 0);
		pp2d_draw_text(2, 2, 0.75, 0.75, BLACK, "What do you want to do?");
		// Draw buttons
		for (int i = (int)(sizeof(buttons)/sizeof(buttons[0]))-1; i >= 0; i--) {
			if (menuSelection == i) {
				// Button is highlighted.
				pp2d_draw_texture(buttontex, buttons[i].x, buttons[i].y);
			} else {
				// Button is not highlighted. Darken the texture.
				pp2d_draw_texture_blend(buttontex, buttons[i].x, buttons[i].y, GRAY);
			}

			// Determine the text height.
			// NOTE: Button texture size is 132x34.
			const int h = 32;

			// Draw the title.
			int y = buttons[i].y + ((34 - h) / 2);
			int w = 0;
			int x = ((2 - w) / 2) + buttons[i].x;
			pp2d_draw_text(x, y, 0.50, 0.50, BLACK, button_titles[i]);
		}
		const int home_width = 144+16;
		const int home_x = (320-home_width)/2;
		pp2d_draw_texture(homeicontex, home_x, 219); // Draw HOME icon
		pp2d_draw_text(home_x+20, 220, 0.50, 0.50, BLACK, ": Return to HOME Menu");
		if (fadealpha > 0) pp2d_draw_rectangle(0, 0, 320, 240, RGBA8(0, 0, 0, fadealpha)); // Fade in/out effect
		pp2d_end_draw();
		
		if (fadein == true) {
			fadealpha -= 15;
			if (fadealpha < 0) {
				fadealpha = 0;
				fadein = false;
			}
		} else if (fadeout == true) {
			fadealpha += 15;
			if (fadealpha > 255) {
				fadealpha = 255;
				fadeout = false;
				if (menuSelection == 0) {
					// Launch DSiMenu++
					while(1) {
						// Buffers for APT_DoApplicationJump().
						u8 param[0x300];
						u8 hmac[0x20];
						// Clear both buffers
						memset(param, 0, sizeof(param));
						memset(hmac, 0, sizeof(hmac));

						APT_PrepareToDoApplicationJump(0, 0x0004801553524C41ULL, MEDIATYPE_NAND);
						// Tell APT to trigger the app launch and set the status of this app to exit
						APT_DoApplicationJump(param, sizeof(param), hmac);
					}
				} else if (menuSelection == 1) {
					// Launch last-ran ROM
					while(1) {
						// Buffers for APT_DoApplicationJump().
						u8 param[0x300];
						u8 hmac[0x20];
						// Clear both buffers
						memset(param, 0, sizeof(param));
						memset(hmac, 0, sizeof(hmac));

						APT_PrepareToDoApplicationJump(0, 0x0004801553524C4EULL, MEDIATYPE_NAND);
						// Tell APT to trigger the app launch and set the status of this app to exit
						APT_DoApplicationJump(param, sizeof(param), hmac);
					}
				}
			}
		}

		if (!fadeout) {
			if ((hDown & KEY_UP) || (hDown & KEY_LEFT)) {
				menuSelection--;
			} else if ((hDown & KEY_DOWN) || (hDown & KEY_RIGHT)) {
				menuSelection++;
			}
			
			if (menuSelection > 1) menuSelection = 0;
			if (menuSelection < 0) menuSelection = 1;
		}
		
		if (hDown & KEY_A) {
			if (!fadein) fadeout = true;
		}
	}

	
	pp2d_exit();

	hidExit();
	srvExit();
	romfsExit();
	sdmcExit();
	aptExit();

    return 0;
}