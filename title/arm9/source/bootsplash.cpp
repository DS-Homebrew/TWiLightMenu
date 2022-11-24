#include "bootsplash.h"
#include <nds.h>
#include <maxmod9.h>

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "graphics/gif.hpp"
#include "graphics/graphics.h"
#include "common/lodepng.h"

#include "sound.h"

extern bool useTwlCfg;

extern bool fadeType;
extern bool controlTopBright;
extern bool controlBottomBright;
extern int screenBrightness;

//extern void loadROMselectAsynch(void);

bool cartInserted;

void bootSplashDSi(void) {
	// u16 whiteCol = ((whiteCol>>10)&0x1f) | ((whiteCol)&((31-3*ms().blfLevel)<<5)) | (whiteCol&(31-6*ms().blfLevel))<<10 | BIT(15);
	toncset16(BG_GFX, 0xFFFF, 256*256);
	toncset16(BG_GFX_SUB, 0xFFFF, 256*256);

	cartInserted = (REG_SCFG_MC != 0x11);

	int language = ms().gameLanguage;
	if (ms().gameLanguage == -1) {
		language = (useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
	}

	char currentDate[16];
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	strftime(currentDate, sizeof(currentDate), "%m/%d", Time);
	bool virtualPain = (strcmp(currentDate, "04/01") == 0 || (ms().getGameRegion() == 0 ? (strcmp(currentDate, "07/21") == 0) : (strcmp(currentDate, "08/14") == 0))); // If April Fools, or the release date of the Virtual Boy
	bool super = (*(u16*)(0x020000C0) == 0x334D || *(u16*)(0x020000C0) == 0x3647 || *(u16*)(0x020000C0) == 0x4353); // Slot-2 flashcard

	bool custom = ms().dsiSplash == 3;

	char path[256];
	if (virtualPain) {
		sprintf(path, "nitro:/video/splash/virtualPain%s.gif", /*ms().wideScreen && !ms().macroMode ? "_wide" :*/ "");
	} else if (ms().dsiSplash == 3 && access("/_nds/TWiLightMenu/extras/splashtop.gif", F_OK) == 0) {
		sprintf(path, "%s:/_nds/TWiLightMenu/extras/splashtop.gif", sys().isRunFromSD() ? "sd" : "fat");
	} else if (ms().macroMode) {
		sprintf(path, "nitro:/video/splash/gameBoy.gif");
	} else if (super) {
		sprintf(path, "nitro:/video/splash/superDS.gif");
	} else {
		sprintf(path, "nitro:/video/splash/%s.gif", language == TWLSettings::ELangChineseS ? "iquedsi" : (sys().isRegularDS() ? "ds" : "dsi"));
	}
	Gif splash(path, true, true);

	path[0] = '\0';
	if (virtualPain) { // Load Virtual Pain image
		strcpy(path, "nitro:/video/hsmsg/virtualPain.gif");
	} else if (ms().dsiSplash == 1) { // Load Touch the Touch Screen to continue image
		sprintf(path, "nitro:/video/tttstc/%i.gif", language);
	} else if (ms().dsiSplash == 2) { // Load H&S image
		sprintf(path, "nitro:/video/hsmsg/%i.gif", language);
	} else if (ms().dsiSplash == 3 && access("/_nds/TWiLightMenu/extras/splashbottom.gif", F_OK) == 0) { // Load custom bottom image
		sprintf(path, "%s:/_nds/TWiLightMenu/extras/splashbottom.gif", sys().isRunFromSD() ? "sd" : "fat");
	}
	Gif healthSafety(path, false, true);

	// For the default splashes, draw first frame, then wait until the top is done
	if (!custom) {
		healthSafety.displayFrame();
		healthSafety.pause();
	}

	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(100), Gif::timerHandler);

	if (cartInserted && !custom) {
		u16 *gfx[2];
		int yPos = virtualPain ? 130 : 142;
		if (ms().macroMode && !virtualPain) {
			yPos = 128;
		}
		for (int i = 0; i < 2; i++) {
			gfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_Bmp);
			oamSet(&oamMain, i, 67 + (i * 64), yPos, 0, 15, SpriteSize_64x32, SpriteColorFormat_Bmp, gfx[i], 0, false, false, false, false, false);
		}

		std::vector<unsigned char> image;
		unsigned int width, height;
		lodepng::decode(image, width, height, virtualPain ? "nitro:/graphics/nintendoPain.png" : "nitro:/graphics/nintendo.png");

		for (unsigned int i = 0, y = 0, x = 0;i < image.size() / 4; i++, x++) {
			if (!virtualPain && image[(i * 4) + 3] > 0) {
				switch (ms().nintendoLogoColor) {
					default: // Gray (Original color)
						break;
					case 1: // Red (Current color)
						image[(i * 4) + 0] = 0xFF;
						image[(i * 4) + 1] = 0;
						image[(i * 4) + 2] = 0;
						break;
					case 2: // Blue (Past JAP color)
						image[(i * 4) + 0] = 0;
						image[(i * 4) + 1] = 0;
						image[(i * 4) + 2] = 0xFF;
						break;
					case 3: // Magneta (GBA color)
						image[(i * 4) + 0] = 0xFF;
						image[(i * 4) + 1] = 0;
						image[(i * 4) + 2] = 0xFF;
						break;
				}
			}
			u16 color = image[i * 4] >> 3 | (image[(i * 4) + 1] >> 3) << 5 | (image[(i * 4) + 2] >> 3) << 10 | (image[(i * 4) + 3] > 0) << 15;
			if (ms().colorMode == 1) {
				color = (color & 0x8000) | (convertVramColorToGrayscale(color) & 0x7FFF);
			}

			if (x >= width) {
				x = 0;
				y++;
			}

			gfx[x >= 64][y * 64 + (x % 64)] = color;
		}

		oamUpdate(&oamMain);
	}

	if (!custom && !virtualPain) {
		controlBottomBright = false;
		fadeType = false;
		screenBrightness = 0;
		swiWaitForVBlank();
		controlTopBright = false;
		screenBrightness = 25;
		swiWaitForVBlank();
	}

	controlBottomBright = true;
	fadeType = true;

	// If both will loop forever, show for 3s or until button press
	if ((!custom && ms().macroMode) || (splash.loopForever() && healthSafety.loopForever())) {
		for (int i = 0; i < 60 * 3 && !keysDown(); i++) {
			swiWaitForVBlank();
			//loadROMselectAsynch();
			scanKeys();

			if (!custom && splash.currentFrame() == 14)
				snd().playDSiBoot();
		}
	} else {
		u16 pressed = 0;
		while (!(splash.finished() && healthSafety.finished()) && !(pressed & KEY_START)) {
			swiWaitForVBlank();
			//loadROMselectAsynch();
			scanKeys();
			pressed = keysDown();
			pressed &= ~KEY_LID;

			if (splash.waitingForInput()) {
				if (!custom && healthSafety.paused())
					healthSafety.unpause();
				if (pressed || ms().dsiSplashAutoSkip) {
					splash.resume();
					snd().playSelect();
				}
			}

			if (healthSafety.waitingForInput()) {
				if (pressed || ms().dsiSplashAutoSkip) {
					healthSafety.resume();
					snd().playSelect();
				}
			}

			if (!custom && splash.currentFrame() == (super ? 1 : 24))
				snd().playDSiBoot();
		}
	}

	// Fade out
	controlTopBright = true;
	controlBottomBright = true;
	fadeType = false;
	for (int i = 0; i < 25; i++) { swiWaitForVBlank(); }

	timerStop(0);
}

void bootSplashInit(void) {
	videoSetMode(MODE_5_2D);
	videoSetModeSub(MODE_5_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	vramSetBankB(VRAM_B_MAIN_SPRITE);
	vramSetBankC(VRAM_C_SUB_BG);

	bgInit(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(3, 3);

	bgInitSub(3, BgType_Bmp8, BgSize_B8_256x256, 0, 0);
	bgSetPriority(7, 3);

	oamInit(&oamMain, SpriteMapping_Bmp_1D_128, false);

	snd();
	bootSplashDSi();
}
