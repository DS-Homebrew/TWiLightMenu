#include "bootsplash.h"
#include <nds.h>
#include <maxmod9.h>

#include "common/dsimenusettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "common/tonccpy.h"
#include "graphics/gif.hpp"
#include "graphics/graphics.h"
#include "graphics/lodepng.h"

#include "soundbank.h"

extern bool useTwlCfg;

extern bool fadeType;
extern bool controlTopBright;
extern bool controlBottomBright;

bool cartInserted;

mm_sound_effect dsiboot;
mm_sound_effect proceed;

void splashSoundInit() {
	mmInitDefaultMem((mm_addr)0x02FA0000);

	mmLoadEffect( SFX_DSIBOOT );
	mmLoadEffect( SFX_SELECT );

	dsiboot = {
		{ SFX_DSIBOOT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};

	proceed = {
		{ SFX_SELECT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};
}

void bootSplashDSi(void) {
	u16 whiteCol = 0xFFFF;
	whiteCol = ((whiteCol>>10)&0x1f) | ((whiteCol)&((31-3*ms().blfLevel)<<5)) | (whiteCol&(31-6*ms().blfLevel))<<10 | BIT(15);
	for (int i = 0; i < 256*256; i++) {
		BG_GFX[i] = whiteCol;
		BG_GFX_SUB[i] = whiteCol;
	}

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
	bool virtualPain = (strcmp(currentDate, "04/01") == 0);

	bool custom = ms().dsiSplash == 3;

	char path[256];
	if (virtualPain) {
		strcpy(path, "nitro:/video/splash/virtualPain.gif");
	} else if (ms().dsiSplash == 3 && access("/_nds/TWiLightMenu/extras/splashtop.gif", F_OK) == 0) {
		sprintf(path, "%s:/_nds/TWiLightMenu/extras/splashtop.gif", sdFound() ? "sd" : "fat");
	} else {
		sprintf(path, "nitro:/video/splash/%s.gif", language == TWLSettings::ELangChineseS ? "iquedsi" : "dsi");
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
		sprintf(path, "%s:/_nds/TWiLightMenu/extras/splashbottom.gif", sdFound() ? "sd" : "fat");
	}
	Gif healthSafety(path, false, true);

	timerStart(0, ClockDivider_1024, TIMER_FREQ_1024(100), Gif::timerHandler);

	// For the default splashes, draw first frame, then wait until the top is done
	if(!custom) {
		while(healthSafety.currentFrame() == 0)
			swiWaitForVBlank();
		healthSafety.pause();
	}

	if (cartInserted && !custom) {
		u16 *gfx[2];
		for(int i = 0; i < 2; i++) {
			gfx[i] = oamAllocateGfx(&oamMain, SpriteSize_64x32, SpriteColorFormat_Bmp);
			oamSet(&oamMain, i, 67 + (i * 64), virtualPain ? 130 : 142, 0, 15, SpriteSize_64x32, SpriteColorFormat_Bmp, gfx[i], 0, false, false, false, false, false);
		}

		std::vector<unsigned char> image;
		unsigned int width, height;
		lodepng::decode(image, width, height, virtualPain ? "nitro:/graphics/nintendoPain.png" : "nitro:/graphics/nintendo.png");

		for(unsigned int i = 0, y = 0, x = 0;i < image.size() / 4; i++, x++) {
			u16 color = image[i * 4] >> 3 | (image[(i * 4) + 1] >> 3) << 5 | (image[(i * 4) + 2] >> 3) << 10 | (image[(i * 4) + 3] > 0) << 15;
			if (ms().colorMode == 1) {
				color = convertVramColorToGrayscale(color);
			}

			if(x >= width) {
				x = 0;
				y++;
			}

			gfx[x >= 64][y * 64 + (x % 64)] = color;
		}

		oamUpdate(&oamMain);
	}

	controlTopBright = true;
	controlBottomBright = true;
	fadeType = true;

	// If both will loop forever, show for 3s or until button press
	if(splash.loopForever() && healthSafety.loopForever()) {
		for (int i = 0; i < 60 * 3 && !keysDown(); i++) {
			swiWaitForVBlank();
			scanKeys();
		}
	} else {
		while (!(splash.finished() && healthSafety.finished())) {
			swiWaitForVBlank();
			scanKeys();
			u16 pressed = keysDown();

			if (splash.waitingForInput()) {
				if(!custom && healthSafety.paused())
					healthSafety.unpause();
				if(pressed) {
					splash.resume();
					mmEffectEx(&proceed);
				}
			}

			if(healthSafety.waitingForInput()) {
				if(pressed) {
					healthSafety.resume();
					mmEffectEx(&proceed);
				}
			}

			if (!custom && splash.currentFrame() == 24)
				mmEffectEx(&dsiboot);
		}
	}

	// Fade out
	controlTopBright = true;
	controlBottomBright = true;
	fadeType = false;
	for (int i = 0; i < 30; i++) { swiWaitForVBlank(); }

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

	splashSoundInit();
	bootSplashDSi();
}
