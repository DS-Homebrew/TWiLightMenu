#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <gl2d.h>

#include "ndsheaderbanner.h"

extern sNDSBannerExt ndsBanner;

typedef enum
{
	GL_FLIP_BOTH	= (1 << 3)
} GL_FLIP_MODE_XTRA;

// Subroutine function signatures arm9
u32 moduleParamsSignature[2]   = {0xDEC00621, 0x2106C0DE};

//
// Look in @data for @find and return the position of it.
//
u32 getOffset(u32* addr, size_t size, u32* find, size_t sizeofFind, int direction)
{
	u32* end = addr + size/sizeof(u32);

    //u32 result = 0;
	bool found = false;

	do {
		for(int i=0;i<(int)sizeofFind;i++) {
			if (addr[i] != find[i]) 
			{
				break;
			} else if(i==(int)sizeofFind-1) {
				found = true;
			}
		}
		if(!found) addr+=direction;
	} while (addr != end && !found);

	if (addr == end) {
		return NULL;
	}

	return (u32)addr;
}

/**
 * Get the title ID.
 * @param ndsFile DS ROM image.
 * @param buf Output buffer for title ID. (Must be at least 4 characters.)
 * @return 0 on success; non-zero on error.
 */
int grabTID(FILE* ndsFile, char *buf) {
	fseek(ndsFile, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
	size_t read = fread(buf, 1, 4, ndsFile);
	return !(read == 4);
}

char arm9binary[0x40000];

/**
 * Get SDK version from an NDS file.
 * @param ndsFile NDS file.
 * @param filename NDS ROM filename.
 * @return 0 on success; non-zero on error.
 */
u32 getSDKVersion(FILE* ndsFile) {
	sNDSHeaderExt NDSHeader;
	fseek(ndsFile, 0, SEEK_SET);
	fread(&NDSHeader, 1, sizeof(NDSHeader), ndsFile);
	
	fseek(ndsFile, NDSHeader.arm9romOffset, SEEK_SET);
	if(NDSHeader.arm9binarySize > 0x40000) NDSHeader.arm9binarySize = 0x40000;
	fread(&arm9binary, 1, NDSHeader.arm9binarySize, ndsFile);

	// Looking for moduleparams
	uint32_t moduleparams = getOffset((u32*)arm9binary, NDSHeader.arm9binarySize, (u32*)moduleParamsSignature, 2, 1);
	if(!moduleparams) {
		return 0;
	}

	return ((module_params_t*)(moduleparams - 0x1C))->sdk_version;
}

// bnriconframeseq[]
static u16 bnriconframeseq[64] = {0x0000};

// bnriconframenum[]
int bnriconPalLine = 0;
int bnriconframenumY = 0;
int bannerFlip = GL_FLIP_NONE;

// bnriconisDSi[]
bool isDirectory = false;
int bnrRomType = 0;
bool bnriconisDSi = false;
bool launchable = true;
bool isHomebrew = false;

/**
 * Get banner sequence from banner file.
 * @param binFile Banner file.
 */
void grabBannerSequence() {
	for (int i = 0; i < 64; i++) {
		bnriconframeseq[i] = ndsBanner.dsi_seq[i];
	}
}

/**
 * Clear loaded banner sequence.
 */
void clearBannerSequence() {
	for (int i = 0; i < 64; i++) {
		bnriconframeseq[i] = 0x0000;
	}
}

static u16 bannerDelayNum = 0x0000;
int currentbnriconframeseq = 0;

/**
 * Play banner sequence.
 * @param binFile Banner file.
 */
void playBannerSequence() {
	if(bnriconframeseq[currentbnriconframeseq] == 0x0001
	&& bnriconframeseq[currentbnriconframeseq+1] == 0x0100) {
		// Do nothing if icon isn't animated
		bnriconPalLine = 0;
		bnriconframenumY = 0;
		bannerFlip = GL_FLIP_NONE;
	} else {
		u16 setframeseq = bnriconframeseq[currentbnriconframeseq];

		// Check for pal line 1
		if((setframeseq >= 0x0000)
		&& (setframeseq < 0x0100)) {
			bnriconPalLine = 0;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0100)
				&& (setframeseq < 0x0200)) {
			bnriconPalLine = 0;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0200)
				&& (setframeseq < 0x0300)) {
			bnriconPalLine = 0;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0300)
				&& (setframeseq < 0x0400)) {
			bnriconPalLine = 0;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0400)
				&& (setframeseq < 0x0500)) {
			bnriconPalLine = 0;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0500)
				&& (setframeseq < 0x0600)) {
			bnriconPalLine = 0;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0600)
				&& (setframeseq < 0x0700)) {
			bnriconPalLine = 0;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0700)
				&& (setframeseq < 0x0800)) {
			bnriconPalLine = 0;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_NONE;
		} else // Check for pal line 2
		if((setframeseq >= 0x0800)
		&& (setframeseq < 0x0900)) {
			bnriconPalLine = 1;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0900)
				&& (setframeseq < 0x0A00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0A00)
				&& (setframeseq < 0x0B00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0B00)
				&& (setframeseq < 0x0C00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0C00)
				&& (setframeseq < 0x0D00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0D00)
				&& (setframeseq < 0x0E00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0E00)
				&& (setframeseq < 0x0F00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x0F00)
				&& (setframeseq < 0x1000)) {
			bnriconPalLine = 1;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_NONE;
		} else // Check for pal line 3
		if((setframeseq >= 0x1000)
		&& (setframeseq < 0x1100)) {
			bnriconPalLine = 2;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1100)
				&& (setframeseq < 0x1200)) {
			bnriconPalLine = 2;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1200)
				&& (setframeseq < 0x1300)) {
			bnriconPalLine = 2;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1300)
				&& (setframeseq < 0x1400)) {
			bnriconPalLine = 2;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1400)
				&& (setframeseq < 0x1500)) {
			bnriconPalLine = 2;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1500)
				&& (setframeseq < 0x1600)) {
			bnriconPalLine = 2;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1600)
				&& (setframeseq < 0x1700)) {
			bnriconPalLine = 2;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1700)
				&& (setframeseq < 0x1800)) {
			bnriconPalLine = 2;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_NONE;
		} else // Check for pal line 4
		if((setframeseq >= 0x1800)
		&& (setframeseq < 0x1900)) {
			bnriconPalLine = 3;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1900)
				&& (setframeseq < 0x1A00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1A00)
				&& (setframeseq < 0x1B00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1B00)
				&& (setframeseq < 0x1C00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1C00)
				&& (setframeseq < 0x1D00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1D00)
				&& (setframeseq < 0x1E00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1E00)
				&& (setframeseq < 0x1F00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x1F00)
				&& (setframeseq < 0x2000)) {
			bnriconPalLine = 3;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_NONE;
		} else // Check for pal line 5
		if((setframeseq >= 0x2000)
		&& (setframeseq < 0x2100)) {
			bnriconPalLine = 4;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2100)
				&& (setframeseq < 0x2200)) {
			bnriconPalLine = 4;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2200)
				&& (setframeseq < 0x2300)) {
			bnriconPalLine = 4;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2300)
				&& (setframeseq < 0x2400)) {
			bnriconPalLine = 4;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2400)
				&& (setframeseq < 0x2500)) {
			bnriconPalLine = 4;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2500)
				&& (setframeseq < 0x2600)) {
			bnriconPalLine = 4;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2600)
				&& (setframeseq < 0x2700)) {
			bnriconPalLine = 4;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2700)
				&& (setframeseq < 0x2800)) {
			bnriconPalLine = 4;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_NONE;
		} else // Check for pal line 6
		if((setframeseq >= 0x2800)
		&& (setframeseq < 0x2900)) {
			bnriconPalLine = 5;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2900)
				&& (setframeseq < 0x2A00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2A00)
				&& (setframeseq < 0x2B00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2B00)
				&& (setframeseq < 0x2C00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2C00)
				&& (setframeseq < 0x2D00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2D00)
				&& (setframeseq < 0x2E00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2E00)
				&& (setframeseq < 0x2F00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x2F00)
				&& (setframeseq < 0x3000)) {
			bnriconPalLine = 5;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_NONE;
		} else // Check for pal line 7
		if((setframeseq >= 0x3000)
		&& (setframeseq < 0x3100)) {
			bnriconPalLine = 6;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3100)
				&& (setframeseq < 0x3200)) {
			bnriconPalLine = 6;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3200)
				&& (setframeseq < 0x3300)) {
			bnriconPalLine = 6;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3300)
				&& (setframeseq < 0x3400)) {
			bnriconPalLine = 6;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3400)
				&& (setframeseq < 0x3500)) {
			bnriconPalLine = 6;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3500)
				&& (setframeseq < 0x3600)) {
			bnriconPalLine = 6;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3600)
				&& (setframeseq < 0x3700)) {
			bnriconPalLine = 6;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3700)
				&& (setframeseq < 0x3800)) {
			bnriconPalLine = 6;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_NONE;
		} else // Check for pal line 8
		if((setframeseq >= 0x3800)
		&& (setframeseq < 0x3900)) {
			bnriconPalLine = 7;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3900)
				&& (setframeseq < 0x3A00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3A00)
				&& (setframeseq < 0x3B00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3B00)
				&& (setframeseq < 0x3C00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3C00)
				&& (setframeseq < 0x3D00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3D00)
				&& (setframeseq < 0x3E00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3E00)
				&& (setframeseq < 0x3F00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_NONE;
		} else if((setframeseq >= 0x3F00)
				&& (setframeseq < 0x4000)) {
			bnriconPalLine = 7;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_NONE;
		} else // Check for horizontally flipped frames
		// Check for pal line 1
		if((setframeseq >= 0x4000)
		&& (setframeseq < 0x4100)) {
			bnriconPalLine = 0;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4100)
				&& (setframeseq < 0x4200)) {
			bnriconPalLine = 0;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4200)
				&& (setframeseq < 0x4300)) {
			bnriconPalLine = 0;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4300)
				&& (setframeseq < 0x4400)) {
			bnriconPalLine = 0;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4400)
				&& (setframeseq < 0x4500)) {
			bnriconPalLine = 0;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4500)
				&& (setframeseq < 0x4600)) {
			bnriconPalLine = 0;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4600)
				&& (setframeseq < 0x4700)) {
			bnriconPalLine = 0;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4700)
				&& (setframeseq < 0x4800)) {
			bnriconPalLine = 0;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_H;
		} else // Check for pal line 2
		if((setframeseq >= 0x4800)
		&& (setframeseq < 0x4900)) {
			bnriconPalLine = 1;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4900)
				&& (setframeseq < 0x4A00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4A00)
				&& (setframeseq < 0x4B00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4B00)
				&& (setframeseq < 0x4C00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4C00)
				&& (setframeseq < 0x4D00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4D00)
				&& (setframeseq < 0x4E00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4E00)
				&& (setframeseq < 0x4F00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x4F00)
				&& (setframeseq < 0x5000)) {
			bnriconPalLine = 1;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_H;
		} else // Check for pal line 3
		if((setframeseq >= 0x5000)
		&& (setframeseq < 0x5100)) {
			bnriconPalLine = 2;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5100)
				&& (setframeseq < 0x5200)) {
			bnriconPalLine = 2;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5200)
				&& (setframeseq < 0x5300)) {
			bnriconPalLine = 2;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5300)
				&& (setframeseq < 0x5400)) {
			bnriconPalLine = 2;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5400)
				&& (setframeseq < 0x5500)) {
			bnriconPalLine = 2;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5500)
				&& (setframeseq < 0x5600)) {
			bnriconPalLine = 2;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5600)
				&& (setframeseq < 0x5700)) {
			bnriconPalLine = 2;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5700)
				&& (setframeseq < 0x5800)) {
			bnriconPalLine = 2;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_H;
		} else // Check for pal line 4
		if((setframeseq >= 0x5800)
		&& (setframeseq < 0x5900)) {
			bnriconPalLine = 3;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5900)
				&& (setframeseq < 0x5A00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5A00)
				&& (setframeseq < 0x5B00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5B00)
				&& (setframeseq < 0x5C00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5C00)
				&& (setframeseq < 0x5D00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5D00)
				&& (setframeseq < 0x5E00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5E00)
				&& (setframeseq < 0x5F00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x5F00)
				&& (setframeseq < 0x6000)) {
			bnriconPalLine = 3;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_H;
		} else // Check for pal line 5
		if((setframeseq >= 0x6000)
		&& (setframeseq < 0x6100)) {
			bnriconPalLine = 4;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6100)
				&& (setframeseq < 0x6200)) {
			bnriconPalLine = 4;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6200)
				&& (setframeseq < 0x6300)) {
			bnriconPalLine = 4;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6300)
				&& (setframeseq < 0x6400)) {
			bnriconPalLine = 4;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6400)
				&& (setframeseq < 0x6500)) {
			bnriconPalLine = 4;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6500)
				&& (setframeseq < 0x6600)) {
			bnriconPalLine = 4;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6600)
				&& (setframeseq < 0x6700)) {
			bnriconPalLine = 4;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6700)
				&& (setframeseq < 0x6800)) {
			bnriconPalLine = 4;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_H;
		} else // Check for pal line 6
		if((setframeseq >= 0x6800)
		&& (setframeseq < 0x6900)) {
			bnriconPalLine = 5;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6900)
				&& (setframeseq < 0x6A00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6A00)
				&& (setframeseq < 0x6B00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6B00)
				&& (setframeseq < 0x6C00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6C00)
				&& (setframeseq < 0x6D00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6D00)
				&& (setframeseq < 0x6E00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6E00)
				&& (setframeseq < 0x6F00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x6F00)
				&& (setframeseq < 0x7000)) {
			bnriconPalLine = 5;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_H;
		} else // Check for pal line 7
		if((setframeseq >= 0x7000)
		&& (setframeseq < 0x7100)) {
			bnriconPalLine = 6;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7100)
				&& (setframeseq < 0x7200)) {
			bnriconPalLine = 6;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7200)
				&& (setframeseq < 0x7300)) {
			bnriconPalLine = 6;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7300)
				&& (setframeseq < 0x7400)) {
			bnriconPalLine = 6;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7400)
				&& (setframeseq < 0x7500)) {
			bnriconPalLine = 6;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7500)
				&& (setframeseq < 0x7600)) {
			bnriconPalLine = 6;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7600)
				&& (setframeseq < 0x7700)) {
			bnriconPalLine = 6;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7700)
				&& (setframeseq < 0x7800)) {
			bnriconPalLine = 6;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_H;
		} else // Check for pal line 8
		if((setframeseq >= 0x7800)
		&& (setframeseq < 0x7900)) {
			bnriconPalLine = 7;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7900)
				&& (setframeseq < 0x7A00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7A00)
				&& (setframeseq < 0x7B00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7B00)
				&& (setframeseq < 0x7C00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7C00)
				&& (setframeseq < 0x7D00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7D00)
				&& (setframeseq < 0x7E00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7E00)
				&& (setframeseq < 0x7F00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_H;
		} else if((setframeseq >= 0x7F00)
				&& (setframeseq < 0x8000)) {
			bnriconPalLine = 7;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_H;
		} else // Check for vertically flipped frames
		// Check for pal line 1
		if((setframeseq >= 0x8000)
		&& (setframeseq < 0x8100)) {
			bnriconPalLine = 0;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8100)
				&& (setframeseq < 0x8200)) {
			bnriconPalLine = 0;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8200)
				&& (setframeseq < 0x8300)) {
			bnriconPalLine = 0;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8300)
				&& (setframeseq < 0x8400)) {
			bnriconPalLine = 0;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8400)
				&& (setframeseq < 0x8500)) {
			bnriconPalLine = 0;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8500)
				&& (setframeseq < 0x8600)) {
			bnriconPalLine = 0;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8600)
				&& (setframeseq < 0x8700)) {
			bnriconPalLine = 0;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8700)
				&& (setframeseq < 0x8800)) {
			bnriconPalLine = 0;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_V;
		} else // Check for pal line 2
		if((setframeseq >= 0x8800)
		&& (setframeseq < 0x8900)) {
			bnriconPalLine = 1;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8900)
				&& (setframeseq < 0x8A00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8A00)
				&& (setframeseq < 0x8B00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8B00)
				&& (setframeseq < 0x8C00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8C00)
				&& (setframeseq < 0x8D00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8D00)
				&& (setframeseq < 0x8E00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8E00)
				&& (setframeseq < 0x8F00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x8F00)
				&& (setframeseq < 0x9000)) {
			bnriconPalLine = 1;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_V;
		} else // Check for pal line 3
		if((setframeseq >= 0x9000)
		&& (setframeseq < 0x9100)) {
			bnriconPalLine = 2;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9100)
				&& (setframeseq < 0x9200)) {
			bnriconPalLine = 2;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9200)
				&& (setframeseq < 0x9300)) {
			bnriconPalLine = 2;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9300)
				&& (setframeseq < 0x9400)) {
			bnriconPalLine = 2;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9400)
				&& (setframeseq < 0x9500)) {
			bnriconPalLine = 2;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9500)
				&& (setframeseq < 0x9600)) {
			bnriconPalLine = 2;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9600)
				&& (setframeseq < 0x9700)) {
			bnriconPalLine = 2;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9700)
				&& (setframeseq < 0x9800)) {
			bnriconPalLine = 2;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_V;
		} else // Check for pal line 4
		if((setframeseq >= 0x9800)
		&& (setframeseq < 0x9900)) {
			bnriconPalLine = 3;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9900)
				&& (setframeseq < 0x9A00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9A00)
				&& (setframeseq < 0x9B00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9B00)
				&& (setframeseq < 0x9C00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9C00)
				&& (setframeseq < 0x9D00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9D00)
				&& (setframeseq < 0x9E00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9E00)
				&& (setframeseq < 0x9F00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0x9F00)
				&& (setframeseq < 0xA000)) {
			bnriconPalLine = 3;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_V;
		} else // Check for pal line 5
		if((setframeseq >= 0xA000)
		&& (setframeseq < 0xA100)) {
			bnriconPalLine = 4;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xA100)
				&& (setframeseq < 0xA200)) {
			bnriconPalLine = 4;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xA200)
				&& (setframeseq < 0xA300)) {
			bnriconPalLine = 4;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xA300)
				&& (setframeseq < 0xA400)) {
			bnriconPalLine = 4;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xA400)
				&& (setframeseq < 0xA500)) {
			bnriconPalLine = 4;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xA500)
				&& (setframeseq < 0xA600)) {
			bnriconPalLine = 4;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xA600)
				&& (setframeseq < 0xA700)) {
			bnriconPalLine = 4;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xA700)
				&& (setframeseq < 0xA800)) {
			bnriconPalLine = 4;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_V;
		} else // Check for pal line 6
		if((setframeseq >= 0xA800)
		&& (setframeseq < 0xA900)) {
			bnriconPalLine = 5;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xA900)
				&& (setframeseq < 0xAA00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xAA00)
				&& (setframeseq < 0xAB00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xAB00)
				&& (setframeseq < 0xAC00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xAC00)
				&& (setframeseq < 0xAD00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xAD00)
				&& (setframeseq < 0xAE00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xAE00)
				&& (setframeseq < 0xAF00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xAF00)
				&& (setframeseq < 0xB000)) {
			bnriconPalLine = 5;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_V;
		} else // Check for pal line 7
		if((setframeseq >= 0xB000)
		&& (setframeseq < 0xB100)) {
			bnriconPalLine = 6;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xB100)
				&& (setframeseq < 0xB200)) {
			bnriconPalLine = 6;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xB200)
				&& (setframeseq < 0xB300)) {
			bnriconPalLine = 6;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xB300)
				&& (setframeseq < 0xB400)) {
			bnriconPalLine = 6;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xB400)
				&& (setframeseq < 0xB500)) {
			bnriconPalLine = 6;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xB500)
				&& (setframeseq < 0xB600)) {
			bnriconPalLine = 6;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xB600)
				&& (setframeseq < 0xB700)) {
			bnriconPalLine = 6;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xB700)
				&& (setframeseq < 0xB800)) {
			bnriconPalLine = 6;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_V;
		} else // Check for pal line 8
		if((setframeseq >= 0xB800)
		&& (setframeseq < 0xB900)) {
			bnriconPalLine = 7;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xB900)
				&& (setframeseq < 0xBA00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xBA00)
				&& (setframeseq < 0xBB00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xBB00)
				&& (setframeseq < 0xBC00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xBC00)
				&& (setframeseq < 0xBD00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xBD00)
				&& (setframeseq < 0xBE00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xBE00)
				&& (setframeseq < 0xBF00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_V;
		} else if((setframeseq >= 0xBF00)
				&& (setframeseq < 0xC000)) {
			bnriconPalLine = 7;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_V;
		} else // Check for both horizontally & vertically flipped frames
		// Check for pal line 1
		if((setframeseq >= 0xC000)
		&& (setframeseq < 0xC100)) {
			bnriconPalLine = 0;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xC100)
				&& (setframeseq < 0xC200)) {
			bnriconPalLine = 0;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xC200)
				&& (setframeseq < 0xC300)) {
			bnriconPalLine = 0;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xC300)
				&& (setframeseq < 0xC400)) {
			bnriconPalLine = 0;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xC400)
				&& (setframeseq < 0xC500)) {
			bnriconPalLine = 0;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xC500)
				&& (setframeseq < 0xC600)) {
			bnriconPalLine = 0;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xC600)
				&& (setframeseq < 0xC700)) {
			bnriconPalLine = 0;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xC700)
				&& (setframeseq < 0xC800)) {
			bnriconPalLine = 0;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_BOTH;
		} else // Check for pal line 2
		if((setframeseq >= 0xC800)
		&& (setframeseq < 0xC900)) {
			bnriconPalLine = 1;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xC900)
				&& (setframeseq < 0xCA00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xCA00)
				&& (setframeseq < 0xCB00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xCB00)
				&& (setframeseq < 0xCC00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xCC00)
				&& (setframeseq < 0xCD00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xCD00)
				&& (setframeseq < 0xCE00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xCE00)
				&& (setframeseq < 0xCF00)) {
			bnriconPalLine = 1;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xCF00)
				&& (setframeseq < 0xD000)) {
			bnriconPalLine = 1;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_BOTH;
		} else // Check for pal line 3
		if((setframeseq >= 0xD000)
		&& (setframeseq < 0xD100)) {
			bnriconPalLine = 2;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xD100)
				&& (setframeseq < 0xD200)) {
			bnriconPalLine = 2;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xD200)
				&& (setframeseq < 0xD300)) {
			bnriconPalLine = 2;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xD300)
				&& (setframeseq < 0xD400)) {
			bnriconPalLine = 2;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xD400)
				&& (setframeseq < 0xD500)) {
			bnriconPalLine = 2;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xD500)
				&& (setframeseq < 0xD600)) {
			bnriconPalLine = 2;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xD600)
				&& (setframeseq < 0xD700)) {
			bnriconPalLine = 2;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xD700)
				&& (setframeseq < 0xD800)) {
			bnriconPalLine = 2;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_BOTH;
		} else // Check for pal line 4
		if((setframeseq >= 0xD800)
		&& (setframeseq < 0xD900)) {
			bnriconPalLine = 3;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xD900)
				&& (setframeseq < 0xDA00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xDA00)
				&& (setframeseq < 0xDB00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xDB00)
				&& (setframeseq < 0xDC00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xDC00)
				&& (setframeseq < 0xDD00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xDD00)
				&& (setframeseq < 0xDE00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xDE00)
				&& (setframeseq < 0xDF00)) {
			bnriconPalLine = 3;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xDF00)
				&& (setframeseq < 0xE000)) {
			bnriconPalLine = 3;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_BOTH;
		} else // Check for pal line 5
		if((setframeseq >= 0xE000)
		&& (setframeseq < 0xE100)) {
			bnriconPalLine = 4;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xE100)
				&& (setframeseq < 0xE200)) {
			bnriconPalLine = 4;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xE200)
				&& (setframeseq < 0xE300)) {
			bnriconPalLine = 4;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xE300)
				&& (setframeseq < 0xE400)) {
			bnriconPalLine = 4;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xE400)
				&& (setframeseq < 0xE500)) {
			bnriconPalLine = 4;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xE500)
				&& (setframeseq < 0xE600)) {
			bnriconPalLine = 4;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xE600)
				&& (setframeseq < 0xE700)) {
			bnriconPalLine = 4;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xE700)
				&& (setframeseq < 0xE800)) {
			bnriconPalLine = 4;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_BOTH;
		} else // Check for pal line 6
		if((setframeseq >= 0xE800)
		&& (setframeseq < 0xE900)) {
			bnriconPalLine = 5;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xE900)
				&& (setframeseq < 0xEA00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xEA00)
				&& (setframeseq < 0xEB00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xEB00)
				&& (setframeseq < 0xEC00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xEC00)
				&& (setframeseq < 0xED00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xED00)
				&& (setframeseq < 0xEE00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xEE00)
				&& (setframeseq < 0xEF00)) {
			bnriconPalLine = 5;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xEF00)
				&& (setframeseq < 0xF000)) {
			bnriconPalLine = 5;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_BOTH;
		} else // Check for pal line 7
		if((setframeseq >= 0xF000)
		&& (setframeseq < 0xF100)) {
			bnriconPalLine = 6;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xF100)
				&& (setframeseq < 0xF200)) {
			bnriconPalLine = 6;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xF200)
				&& (setframeseq < 0xF300)) {
			bnriconPalLine = 6;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xF300)
				&& (setframeseq < 0xF400)) {
			bnriconPalLine = 6;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xF400)
				&& (setframeseq < 0xF500)) {
			bnriconPalLine = 6;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xF500)
				&& (setframeseq < 0xF600)) {
			bnriconPalLine = 6;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xF600)
				&& (setframeseq < 0xF700)) {
			bnriconPalLine = 6;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xF700)
				&& (setframeseq < 0xF800)) {
			bnriconPalLine = 6;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_BOTH;
		} else // Check for pal line 8
		if((setframeseq >= 0xF800)
		&& (setframeseq < 0xF900)) {
			bnriconPalLine = 7;
			bnriconframenumY = 0;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xF900)
				&& (setframeseq < 0xFA00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 1;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xFA00)
				&& (setframeseq < 0xFB00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 2;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xFB00)
				&& (setframeseq < 0xFC00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 3;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xFC00)
				&& (setframeseq < 0xFD00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 4;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xFD00)
				&& (setframeseq < 0xFE00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 5;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xFE00)
				&& (setframeseq < 0xFF00)) {
			bnriconPalLine = 7;
			bnriconframenumY = 6;
			bannerFlip = GL_FLIP_BOTH;
		} else if((setframeseq >= 0xFF00)
				&& (setframeseq < 0x10000)) {
			bnriconPalLine = 7;
			bnriconframenumY = 7;
			bannerFlip = GL_FLIP_BOTH;
		}

		bannerDelayNum++;
		if(bannerDelayNum >= (setframeseq & 0x00FF)) {
			bannerDelayNum = 0x0000;
			currentbnriconframeseq++;
			if(bnriconframeseq[currentbnriconframeseq] == 0x0000) {
				currentbnriconframeseq = 0;	// Reset sequence
			}
		}

	}
}
