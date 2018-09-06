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
	fseek(ndsFile, offsetof(sNDSHeadertitlecodeonly, gameCode), SEEK_SET);
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
static u16 bnriconframeseq[40][64] = {0x0000};

// bnriconframenum[]
int bnriconPalLine[40] = {0};
int bnriconframenumY[40] = {0};
int bannerFlip[40] = {GL_FLIP_NONE};

// bnriconisDSi[]
bool isDirectory[40] = {false};
int bnrRomType[40] = {0};
bool bnriconisDSi[40] = {false};
int bnrWirelessIcon[40] = {0}; // 0 = None, 1 = Local, 2 = WiFi
bool isDSiWare[40] = {true};
int isHomebrew[40] = {0}; // 0 = No, 1 = Yes with no DSi-Extended header, 2 = Yes with DSi-Extended header

/**
 * Get banner sequence from banner file.
 * @param binFile Banner file.
 */
void grabBannerSequence(int iconnum)
{
	for (int i = 0; i < 64; i++)
	{
		bnriconframeseq[iconnum][i] = ndsBanner.dsi_seq[i];
	}
}

/**
 * Clear loaded banner sequence.
 */
void clearBannerSequence(int iconnum)
{
	for (int i = 0; i < 64; i++)
	{
		bnriconframeseq[iconnum][i] = 0x0000;
	}
}

static u16 bannerDelayNum[40] = {0x0000};
int currentbnriconframeseq[40] = {0};

/**
 * Play banner sequence.
 * @param binFile Banner file.
 */
void playBannerSequence(int iconnum)
{
	if (bnriconframeseq[iconnum][currentbnriconframeseq[iconnum]] == 0x0001 
	&& bnriconframeseq[iconnum][currentbnriconframeseq[iconnum] + 1] == 0x0100)
	{
		// Do nothing if icon isn't animated
		bnriconPalLine[iconnum] = 0;
		bnriconframenumY[iconnum] = 0;
		bannerFlip[iconnum] = GL_FLIP_NONE;
	}
	else
	{
		u16 setframeseq = bnriconframeseq[iconnum][currentbnriconframeseq[iconnum]];
		bnriconPalLine[iconnum] = SEQ_PAL(setframeseq);
		bnriconframenumY[iconnum] = SEQ_BMP(setframeseq);
		bool flipH = SEQ_FLIPH(setframeseq);
		bool flipV = SEQ_FLIPV(setframeseq);

		if (flipH && flipV)
		{
			bannerFlip[iconnum] = GL_FLIP_BOTH;
		}
		else if (!flipH && !flipV)
		{
			bannerFlip[iconnum] = GL_FLIP_NONE;
		}
		else if (flipH && !flipV)
		{
			bannerFlip[iconnum] = GL_FLIP_H;
		}
		else if (!flipH && flipV)
		{
			bannerFlip[iconnum] = GL_FLIP_V;
		}

		bannerDelayNum[iconnum]++;
		if (bannerDelayNum[iconnum] >= (setframeseq & 0x00FF))
		{
			bannerDelayNum[iconnum] = 0x0000;
			currentbnriconframeseq[iconnum]++;
			if (bnriconframeseq[iconnum][currentbnriconframeseq[iconnum]] == 0x0000)
			{
				currentbnriconframeseq[iconnum] = 0; // Reset sequence
			}
		}
	}
}
