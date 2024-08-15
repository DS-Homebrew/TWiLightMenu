#include <nds.h>
#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include <gl2d.h>

#include "ndsheaderbanner.h"
#include "module_params.h"

static u32 arm9Sig[3][4];

extern sNDSBannerExt ndsBanner;

char gameTid[2][5] = {0};
u8 romVersion[2] = {0};
u8 unitCode[2] = {0};
u16 headerCRC[2] = {0};

// Needed to test if homebrew
char tidBuf[4];

bool checkDsiBinaries(FILE* ndsFile) {
	sNDSHeaderExt ndsHeader;

	fseek(ndsFile, 0, SEEK_SET);
	fread(&ndsHeader, 1, sizeof(ndsHeader), ndsFile);

	if (ndsHeader.unitCode == 0) {
		return true;
	}

	if (ndsHeader.arm9iromOffset < 0x8000 || ndsHeader.arm9iromOffset >= 0x20000000
	 || ndsHeader.arm7iromOffset < 0x8000 || ndsHeader.arm7iromOffset >= 0x20000000) {
		return false;
	}

	for (int i = 0; i < 3; i++) {
		arm9Sig[i][0] = 0;
		arm9Sig[i][1] = 0;
		arm9Sig[i][2] = 0;
		arm9Sig[i][3] = 0;
	}

	fseek(ndsFile, 0x8000, SEEK_SET);
	fread(arm9Sig[0], sizeof(u32), 4, ndsFile);
	fseek(ndsFile, ndsHeader.arm9iromOffset, SEEK_SET);
	fread(arm9Sig[1], sizeof(u32), 4, ndsFile);
	fseek(ndsFile, ndsHeader.arm7iromOffset, SEEK_SET);
	fread(arm9Sig[2], sizeof(u32), 4, ndsFile);
	for (int i = 1; i < 3; i++) {
		if (arm9Sig[i][0] == arm9Sig[0][0]
		 && arm9Sig[i][1] == arm9Sig[0][1]
		 && arm9Sig[i][2] == arm9Sig[0][2]
		 && arm9Sig[i][3] == arm9Sig[0][3]) {
			return false;
		}
		if (arm9Sig[i][0] == 0
		 && arm9Sig[i][1] == 0
		 && arm9Sig[i][2] == 0
		 && arm9Sig[i][3] == 0) {
			return false;
		}
		if (arm9Sig[i][0] == 0xFFFFFFFF
		 && arm9Sig[i][1] == 0xFFFFFFFF
		 && arm9Sig[i][2] == 0xFFFFFFFF
		 && arm9Sig[i][3] == 0xFFFFFFFF) {
			return false;
		}
	}

	return true;
}

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
int bnriconPalLoaded[2]{};
int bnriconPalLine[2]{};
int bnriconPalLinePrev[2]{};
int bnriconframenumY[2]{};
int bnriconframenumYPrev[2]{};
int bannerFlip[2]{GL_FLIP_NONE, GL_FLIP_NONE};
int bannerFlipPrev[2]{GL_FLIP_NONE, GL_FLIP_NONE};

// bnriconisDSi[]
bool isDirectory[2]{false, false};
eROMType bnrRomType[2]{};
bool bnriconisDSi[2]{false, false};
int bnrWirelessIcon[2]{}; 			// 0 = None, 1 = Local, 2 = WiFi
bool isDSiWare[2]{false, false};
bool isHomebrew[2]{false, false};
bool isModernHomebrew[2]{false, false};		// false == No DSi-Extended header, true == Has DSi-Extended header
int customIcon[2]{};				// 0 = None, 1 = png, 2 = banner.bin, -1 = error
char customIconPath[256];

/**
 * Get banner sequence from banner file.
 * @param binFile Banner file.
 */
void grabBannerSequence(int num)
{
	for (int i = 0; i < 64; i++) {
		bnriconframeseq[num][i] = ndsBanner.dsi_seq[i];
	}
}

/**
 * Clear loaded banner sequence.
 */
void clearBannerSequence(int num)
{
	for (int i = 0; i < 64; i++) {
		bnriconframeseq[num][i] = 0x0000;
	}
}

static u16 bannerDelayNum[2] = {0x0000};
int currentbnriconframeseq[2] = {0};

/**
 * Play banner sequence.
 * @param binFile Banner file.
 */
bool playBannerSequence(int num)
{
	if (bnriconframeseq[num][currentbnriconframeseq[num] + 1] == 0x0100) {
		// Do nothing if icon isn't animated
		bnriconPalLine[num] = 0;
		bnriconframenumY[num] = 0;
		bannerFlip[num] = GL_FLIP_NONE;
	} else {
		u16 setframeseq = bnriconframeseq[num][currentbnriconframeseq[num]];
		bnriconPalLine[num] = SEQ_PAL(setframeseq);
		bnriconframenumY[num] =  SEQ_BMP(setframeseq);
		bool flipH = SEQ_FLIPH(setframeseq);
		bool flipV = SEQ_FLIPV(setframeseq);

		if (flipH && flipV) {
			bannerFlip[num] = GL_FLIP_H | GL_FLIP_V;
		} else if (!flipH && !flipV) {
			bannerFlip[num] = GL_FLIP_NONE;
		} else if (flipH && !flipV) {
			bannerFlip[num] = GL_FLIP_H;
		} else if (!flipH && flipV) {
			bannerFlip[num] = GL_FLIP_V;
		}

		bool updateIcon = false;

		if (bnriconPalLinePrev[num] != bnriconPalLine[num]) {
			bnriconPalLinePrev[num] = bnriconPalLine[num];
			updateIcon = true;
		}

		if (bnriconframenumYPrev[num] != bnriconframenumY[num]) {
			bnriconframenumYPrev[num] = bnriconframenumY[num];
			updateIcon = true;
		}

		if (bannerFlipPrev[num] != bannerFlip[num]) {
			bannerFlipPrev[num] = bannerFlip[num];
			updateIcon = true;
		}

		bannerDelayNum[num]++;
		if (bannerDelayNum[num] >= (setframeseq & 0x00FF)) {
			bannerDelayNum[num] = 0x0000;
			currentbnriconframeseq[num]++;
			if (bnriconframeseq[num][currentbnriconframeseq[num]] == 0x0000) {
				currentbnriconframeseq[num] = 0; // Reset sequence
			}
		}

		return updateIcon;
	}

	return false;
}
