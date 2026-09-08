#ifndef DSI_BANNER_H
#define DSI_BANNER_H

#include <nds/ndstypes.h>



#define DSI_SUBBANNER_FILE_SIZE		0x4000	//!< Size of a banner save file.
#define DSI_SUBBANNER_ANIME_OFF		0x0020	//!< Animation block offset in the file.
#define DSI_SUBBANNER_VER		0x0103	//!< version 3 + platform 1, read as a u16.

#define DSI_BANNER_ANIME_SIZE		0x1180	//!< dsi_icon + dsi_palette + dsi_seq.
#define DSI_BANNER_SEQ_LEN		64	//!< Animation sequence entries.

// Animation control token. A frame count of 0
// terminates the sequence: cell 0 means loop, cell 1 means hold the last frame.
#define DSI_SEQ_FRAMES(t)	((t) & 0x00FF)
#define DSI_SEQ_CELL(t)		(((t) >> 8) & 0x07)
#define DSI_SEQ_PLTT(t)		(((t) >> 11) & 0x07)
#define DSI_SEQ_FLIPH(t)	(((t) >> 14) & 0x01)
#define DSI_SEQ_FLIPV(t)	(((t) >> 15) & 0x01)

//! One resolved animation frame.
typedef struct {
	u8 cell;	//!< Bitmap index into dsi_icon[].
	u8 pltt;	//!< Palette index into dsi_palette[].
	bool flipH;
	bool flipV;
} DsiBannerFrame;

/**
 * Advance a banner animation sequence by one video frame.
 * @param seq Sequence table, DSI_BANNER_SEQ_LEN entries.
 * @param pos Current entry index, updated in place.
 * @param delay Frames the current entry has been shown for, updated in place.
 * @param out Receives the frame to display.
 * @return false if the sequence is empty (out is cleared), otherwise true.
 */
bool dsiBannerSeqStep(const u16 *seq, int *pos, u16 *delay, DsiBannerFrame *out);

/**
 * Read a sub-banner file, and validate it in full before it is applied.
 * Nothing is written unless the file is valid, so a blank, truncated or corrupt
 * banner save can never damage the banner already loaded from the ROM.
 * @param path Banner save file to read.
 * @param animeOut Receives DSI_BANNER_ANIME_SIZE bytes (&sNDSBannerExt::dsi_icon).
 * @param crcOut Receives the file's crc16_anime (-> sNDSBannerExt::crc[3]).
 * @return true if a valid sub-banner was loaded.
 */
bool dsiSubBannerLoad(const char *path, void *animeOut, u16 *crcOut);

#endif // DSI_BANNER_H
