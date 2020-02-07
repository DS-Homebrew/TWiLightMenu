#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include "common/gl2d.h"

#include "ndsheaderbanner.h"
#include "module_params.h"

extern sNDSBannerExt ndsBanner;

// Needed to test if homebrew
char tidBuf[4];

typedef enum
{
	GL_FLIP_BOTH = (1 << 3)
} GL_FLIP_MODE_XTRA;

/**
 * Get the title ID.
 * @param ndsFile DS ROM image.
 * @param buf Output buffer for title ID. (Must be at least 4 characters.)
 * @return 0 on success; non-zero on error.
 */
int grabTID(FILE *ndsFile, char *buf)
{
	fseek(ndsFile, offsetof(sNDSHeaderExt, gameCode), SEEK_SET);
	size_t read = fread(buf, 1, 4, ndsFile);
	return !(read == 4);
}

/**
 * Get SDK version from an NDS file.
 * @param ndsFile NDS file.
 * @param filename NDS ROM filename.
 * @return 0 on success; non-zero on error.
 */
u32 getSDKVersion(FILE *ndsFile)
{
	sNDSHeaderExt NDSHeader;
	fseek(ndsFile, 0, SEEK_SET);
	fread(&NDSHeader, 1, sizeof(NDSHeader), ndsFile);
	if (NDSHeader.arm7destination >= 0x037F8000 || grabTID(ndsFile, tidBuf) != 0)
		return 0;
	return getModuleParams(&NDSHeader, ndsFile)->sdk_version;
}

// bnriconframeseq[]
static u16 bnriconframeseq[2][64] = {0x0000};

// bnriconframenum[]
int bnriconPalLine[2] = {0};
int bnriconframenumY[2] = {0};
int bannerFlip[2] = {GL_FLIP_NONE};

// bnriconisDSi[]
bool isDirectory = false;
int bnrRomType = 0;
bool bnriconisDSi[2] = {false};
int bnrWirelessIcon[2] = {0}; 			// 0 = None, 1 = Local, 2 = WiFi
bool isDSiWare = false;
bool isHomebrew = false;
bool isModernHomebrew = false;		// false == No DSi-Extended header, true == Has DSi-Extended header

/**
 * Get banner sequence from banner file.
 * @param binFile Banner file.
 */
void grabBannerSequence(int num)
{
	for (int i = 0; i < 64; i++)
	{
		bnriconframeseq[num][i] = ndsBanner.dsi_seq[i];
	}
}

/**
 * Clear loaded banner sequence.
 */
void clearBannerSequence(int num)
{
	for (int i = 0; i < 64; i++)
	{
		bnriconframeseq[num][i] = 0x0000;
	}
}

static u16 bannerDelayNum[2] = {0x0000};
int currentbnriconframeseq[2] = {0};

/**
 * Play banner sequence.
 * @param binFile Banner file.
 */
void playBannerSequence(int num)
{
	if (bnriconframeseq[num][currentbnriconframeseq[num]] == 0x0001 && bnriconframeseq[num][currentbnriconframeseq[num] + 1] == 0x0100)
	{
		// Do nothing if icon isn't animated
		bnriconPalLine[num] = 0;
		bnriconframenumY[num] = 0;
		bannerFlip[num] = GL_FLIP_NONE;
	}
	else
	{
		u16 setframeseq = bnriconframeseq[num][currentbnriconframeseq[num]];
		bnriconPalLine[num] = SEQ_PAL(setframeseq);
		bnriconframenumY[num] =  SEQ_BMP(setframeseq);
		bool flipH = SEQ_FLIPH(setframeseq);
		bool flipV = SEQ_FLIPV(setframeseq);

		if (flipH && flipV)
		{
			bannerFlip[num] = GL_FLIP_BOTH;
		}
		else if (!flipH && !flipV)
		{
			bannerFlip[num] = GL_FLIP_NONE;
		}
		else if (flipH && !flipV)
		{
			bannerFlip[num] = GL_FLIP_H;
		}
		else if (!flipH && flipV)
		{
			bannerFlip[num] = GL_FLIP_V;
		}

		bannerDelayNum[num]++;
		if (bannerDelayNum[num] >= (setframeseq & 0x00FF))
		{
			bannerDelayNum[num] = 0x0000;
			currentbnriconframeseq[num]++;
			if (bnriconframeseq[num][currentbnriconframeseq[num]] == 0x0000)
			{
				currentbnriconframeseq[num] = 0; // Reset sequence
			}
		}
	}
}
