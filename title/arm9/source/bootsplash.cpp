#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <maxmod9.h>

#include "common/lzss.h"
#include "common/tonccpy.h"
#include "common/dsimenusettings.h"
#include "common/systemdetails.h"

#include "soundbank.h"
//#include "soundbank_bin.h"

extern bool useTwlCfg;

extern u16 bmpImageBuffer[256*192];
extern u16 videoImageBuffer[39][256*144];

extern u16 convertToDsBmp(u16 val);

extern void* dsiSplashLocation;

extern bool fadeType;
extern bool controlTopBright;
extern bool controlBottomBright;
extern int screenBrightness;

bool cartInserted;

static char videoFrameFilename[256];

static FILE* videoFrameFile;

extern bool rocketVideo_playVideo;
extern bool rocketVideo_playBackwards;
extern bool rocketVideo_screen;
extern int rocketVideo_videoYpos;
extern int rocketVideo_videoYsize;
extern int rocketVideo_videoFrames;
extern int rocketVideo_videoFps;
extern int rocketVideo_currentFrame;

#include "bootsplash.h"

#define CONSOLE_SCREEN_WIDTH 32
#define CONSOLE_SCREEN_HEIGHT 24

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

void drawNintendoLogoToVram(void) {
	if (!cartInserted) return;

	// Draw first half of Nintendo logo
	int x = 66;
	int y = 155;
	for (int i=122*14; i<122*28; i++) {
		if (x >= 66+122) {
			x = 66;
			y--;
		}
		if (BG_GFX[(256*192)+i] != 0xFFFF) {
			BG_GFX[y*256+x] = BG_GFX[(256*192)+i];
		}
		x++;
	}
}

void BootSplashDSi(void) {

	u16 whiteCol = 0xFFFF;
	whiteCol = ((whiteCol>>10)&0x1f) | ((whiteCol)&((31-3*ms().blfLevel)<<5)) | (whiteCol&(31-6*ms().blfLevel))<<10 | BIT(15);
	for (int i = 0; i < 256*256; i++) {
		BG_GFX[i] = whiteCol;
		BG_GFX_SUB[i] = whiteCol;
	}

	cartInserted = (REG_SCFG_MC != 0x11);

	int language = ms().guiLanguage;
	if (ms().guiLanguage == -1) {
		language = (useTwlCfg ? *(u8*)0x02000406 : PersonalData->language);
	}

	char currentDate[16];
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	strftime(currentDate, sizeof(currentDate), "%m/%d", Time);

	bool virtualPain = (strcmp(currentDate, "04/01") == 0);

	if (virtualPain) {
		videoFrameFile = fopen("nitro:/graphics/VirtualPain.bmp", "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 1, 0x18000, videoFrameFile);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 191;
			for (int i=0; i<256*192; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				BG_GFX[y*256+x] = convertToDsBmp(val);
				x++;
			}
		}
		fclose(videoFrameFile);

		if (cartInserted) {
			videoFrameFile = fopen("nitro:/graphics/nintendoPain.bmp", "rb");

			if (videoFrameFile) {
				// Start loading
				fseek(videoFrameFile, 0xe, SEEK_SET);
				u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
				fseek(videoFrameFile, pixelStart, SEEK_SET);
				fread(bmpImageBuffer, 1, 0x1D00, videoFrameFile);
				u16* src = bmpImageBuffer;
				int x = 67;
				int y = 159;
				for (int i=0; i<122*30; i++) {
					if (x >= 67+122) {
						x = 67;
						y--;
					}
					u16 val = *(src++);
					BG_GFX[y*256+x] = convertToDsBmp(val);
					x++;
				}
			}
			fclose(videoFrameFile);
		}

		controlTopBright = true;
		controlBottomBright = true;
	} else {
		controlBottomBright = false;
	}

	if (ms().dsiSplash == 2) {
		// Load H&S image
		//Get the language for the splash screen
		sprintf(videoFrameFilename, (virtualPain ? "nitro:/graphics/VirtualPain_bot.bmp" : "nitro:/graphics/hsmsg%i.bmp"), language);
		FILE* file = fopen(videoFrameFilename, "rb");

		if (file) {
			// Start loading
			fseek(file, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(file) + 0xe;
			fseek(file, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 1, 0x18000, file);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 191;
			for (int i=0; i<256*192; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				BG_GFX_SUB[y*256+x] = convertToDsBmp(val);
				x++;
			}
		}

		fclose(file);
	}

	bool sixtyFps = true;

	fadeType = true;

	if (cartInserted) {
		videoFrameFile = fopen("nitro:/graphics/nintendo.bmp", "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 1, 0x1B00, videoFrameFile);
			u16* src = bmpImageBuffer;
			for (int i=0; i<122*28; i++) {
				u16 val = *(src++);
				if (val != 0x7C1F) {
					BG_GFX[(256*192)+i] = convertToDsBmp(val);
				}
			}
		}
		fclose(videoFrameFile);
	}

	if (!virtualPain) {
	if (sixtyFps) {
		rocketVideo_videoFrames = 108;
		rocketVideo_videoFps = 60;

		bool dsiSixtyFpsRead = (isDSiMode() || REG_SCFG_EXT != 0);

		videoFrameFile = fopen(language==6 ? "nitro:/video/iquedsisplash_60fps.lz77.rvid" : "nitro:/video/dsisplash_60fps.lz77.rvid", "rb");

		/*for (u8 selectedFrame = 0; selectedFrame <= rocketVideo_videoFrames; selectedFrame++) {
			if (selectedFrame < 0x10) {
				snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/dsisplash_60fps/0x0%x.bmp", (int)selectedFrame);
			} else {
				snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/dsisplash_60fps/0x%x.bmp", (int)selectedFrame);
			}
			videoFrameFile = fopen(videoFrameFilename, "rb");

			if (videoFrameFile) {
				// Start loading
				fseek(videoFrameFile, 0xe, SEEK_SET);
				u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
				fseek(videoFrameFile, pixelStart, SEEK_SET);
				fread(bmpImageBuffer, 2, 0x12000, videoFrameFile);
				u16* src = bmpImageBuffer;
				int x = 0;
				int y = 143;
				for (int i=0; i<256*144; i++) {
					if (x >= 256) {
						x = 0;
						y--;
					}
					u16 val = *(src++);
					videoImageBuffer[0][y*256+x] = convertToDsBmp(val);
					x++;
				}
			}
			fclose(videoFrameFile);
			memcpy(dsiSplashLocation+(selectedFrame*0x12000), videoImageBuffer[0], 0x12000);
		}*/

		if (videoFrameFile) {
			bool doRead = false;

			if (dsiSixtyFpsRead) {
				doRead = true;
			} /*else if (sys().isRegularDS()) {
				fseek(videoFrameFile, 0x200, SEEK_SET);
				sysSetCartOwner (BUS_OWNER_ARM9);	// Allow arm9 to access GBA ROM (or in this case, the DS Memory Expansion Pak)
				*(vu32*)(0x08240000) = 1;
				if (*(vu32*)(0x08240000) == 1) {
					// Set to load video into DS Memory Expansion Pak
					dsiSplashLocation = (void*)0x09000000;
					doRead = true;
				}
			}*/
			if (doRead) {
				fread(videoImageBuffer, 1, 0x108000, videoFrameFile);
				LZ77_Decompress((u8*)videoImageBuffer, (u8*)dsiSplashLocation);
			} else {
				sixtyFps = false;
			}
		} else {
			sixtyFps = false;
		}
		fclose(videoFrameFile);
	}
	if (!sixtyFps) {
		rocketVideo_videoFrames = 42;
		rocketVideo_videoFps = 24;

		videoFrameFile = fopen("nitro:/video/dsisplash.rvid", "rb");
		fseek(videoFrameFile, 0x200, SEEK_SET);

		for (int selectedFrame = 0; selectedFrame < 39; selectedFrame++) {
			fread(videoImageBuffer[selectedFrame], 1, 0x12000, videoFrameFile);

			if (cartInserted && selectedFrame > 5) {
				// Draw first half of Nintendo logo
				int x = 66;
				int y = 130+13;
				for (int i=122*14; i<122*28; i++) {
					if (x >= 66+122) {
						x = 66;
						y--;
					}
					if (BG_GFX[(256*192)+i] != 0xFFFF) {
						videoImageBuffer[selectedFrame][y*256+x] = BG_GFX[(256*192)+i];
					}
					x++;
				}
			}
		}
	}
		controlTopBright = false;
		fadeType = false;
		screenBrightness = 20;
		swiWaitForVBlank();

		rocketVideo_videoYpos = 12;
		rocketVideo_videoYsize = 144;
		rocketVideo_screen = true;
		rocketVideo_playVideo = !virtualPain;
		controlBottomBright = true;
		fadeType = true;

	while (rocketVideo_playVideo) {
		if (sixtyFps && rocketVideo_currentFrame >= 16) {
			drawNintendoLogoToVram();
		}
		if (cartInserted && rocketVideo_currentFrame == (sixtyFps ? 16 : 6)) {
			// Draw last half of Nintendo logo
			int x = 66;
			int y = 144+13;
			for (int i=0; i<122*14; i++) {
				if (x >= 66+122) {
					x = 66;
					y--;
				}
				BG_GFX[(y+12)*256+x] = BG_GFX[(256*192)+i];
				x++;
			}
		}
		if (rocketVideo_currentFrame == (sixtyFps ? 24 : 10)) {
			mmEffectEx(&dsiboot);
			break;
		}
		swiWaitForVBlank();
	}

	if (!sixtyFps) {
		for (int selectedFrame = 39; selectedFrame <= 42; selectedFrame++) {
			fread(videoImageBuffer[selectedFrame-39], 1, 0x12000, videoFrameFile);

			if (cartInserted) {
				// Draw first half of Nintendo logo
				int x = 66;
				int y = 130+13;
				for (int i=122*14; i<122*28; i++) {
					if (x >= 66+122) {
						x = 66;
						y--;
					}
					if (BG_GFX[(256*192)+i] != 0xFFFF) {
						videoImageBuffer[selectedFrame-39][y*256+x] = BG_GFX[(256*192)+i];
					}
					x++;
				}
			}
		}
		fclose(videoFrameFile);
	}

	while (rocketVideo_playVideo) {
		if (sixtyFps && rocketVideo_currentFrame >= 16) {
			drawNintendoLogoToVram();
		}
		swiWaitForVBlank();
	}
	if (!sixtyFps) swiWaitForVBlank();

	for (int i = 0; i < 256*59; i++) {
		BG_GFX[i] = whiteCol;
	}
	}

	rocketVideo_videoFrames = 29;
	rocketVideo_videoFps = 60;

	for (u8 selectedFrame = 0; selectedFrame <= rocketVideo_videoFrames; selectedFrame++) {
		if (selectedFrame < 0x10) {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/tttstc_%i/0x0%x.bmp", (int)language, (int)selectedFrame);
		} else {
			snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/video/tttstc_%i/0x%x.bmp", (int)language, (int)selectedFrame);
		}
		videoFrameFile = fopen(videoFrameFilename, "rb");

		if (videoFrameFile) {
			// Start loading
			fseek(videoFrameFile, 0xe, SEEK_SET);
			u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
			fseek(videoFrameFile, pixelStart, SEEK_SET);
			fread(bmpImageBuffer, 1, 0x4000, videoFrameFile);
			u16* src = bmpImageBuffer;
			int x = 0;
			int y = 31;
			for (int i=0; i<256*32; i++) {
				if (x >= 256) {
					x = 0;
					y--;
				}
				u16 val = *(src++);
				videoImageBuffer[selectedFrame][y*256+x] = convertToDsBmp(val);
				x++;
			}
		}
		fclose(videoFrameFile);
	}

	rocketVideo_videoYpos = (ms().dsiSplash == 2 ? 160 : 80);
	rocketVideo_videoYsize = 32;
	rocketVideo_screen = false;
	rocketVideo_playVideo = true;
	int touchToContinueWait = 59;
	int touchToContinue_secondsWaited = -1;

	while (1) {
		touchToContinueWait++;
		if (!rocketVideo_playVideo) {
			rocketVideo_playBackwards = !rocketVideo_playBackwards;
			if (rocketVideo_playBackwards) {
				rocketVideo_currentFrame = rocketVideo_videoFrames+1;
			} else {
				rocketVideo_currentFrame = -1;
			}
			rocketVideo_playVideo = true;
		}
		if (touchToContinueWait == 60) {
			touchToContinueWait = 0;
			touchToContinue_secondsWaited++;
		}
		scanKeys();
		if ((keysDown() & (KEY_L | KEY_R
				| KEY_A | KEY_B | KEY_X | KEY_Y
				| KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT
				| KEY_START | KEY_SELECT | KEY_TOUCH)) || (touchToContinue_secondsWaited == 60)) {
			mmEffectEx(&proceed);
			break;
		}
		swiWaitForVBlank();
	}

	/*FILE* destinationFile = fopen("sd:/_nds/TWiLightMenu/extractedvideo.rvid", "wb");
	fwrite(dsiSplashLocation, 1, 0x7C0000, destinationFile);
	fclose(destinationFile);*/

	// Fade out
	controlTopBright = true;
	controlBottomBright = true;
	fadeType = false;
	for (int i = 0; i < 30; i++) { swiWaitForVBlank(); }

	rocketVideo_playVideo = false;
}

void BootSplashInit(void) {

	videoSetMode(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	REG_BG3CNT = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X = 0;
	REG_BG3Y = 0;
	REG_BG3PA = 1<<8;
	REG_BG3PB = 0;
	REG_BG3PC = 0;
	REG_BG3PD = 1<<8;

	REG_BG3CNT_SUB = BG_MAP_BASE(0) | BG_BMP16_256x256 | BG_PRIORITY(0);
	REG_BG3X_SUB = 0;
	REG_BG3Y_SUB = 0;
	REG_BG3PA_SUB = 1<<8;
	REG_BG3PB_SUB = 0;
	REG_BG3PC_SUB = 0;
	REG_BG3PD_SUB = 1<<8;

	splashSoundInit();
	BootSplashDSi();
}
