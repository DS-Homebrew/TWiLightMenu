#include "ndsheader.h"
#ifndef __FIXED_BANNERS_H__
#define __FIXED_BANNERS_H__

inline void fixBanner(sNDSBannerExt *banner)
{
    // Check for fixed banner here.
    u16 crc1 = banner->crc[0];
	u16 crc2 = banner->crc[1];
	u16 crc3 = banner->crc[2];
	u16 crc4 = banner->crc[3];
    std::string fixedBannerPath;

    if (crc4 == 0xD8F4) {
		// Fire Emblem - Heroes of Light and Shadow
        fixedBannerPath = "nitro:/fixedbanners/Fire Emblem - Heroes of Light and Shadow (J) (Eng).bnr";
	} else if (crc4 == 0xEE5D && crc1 != 0x4683 && crc1 != 0xA251) {
        // Pokemon Black Version
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Black Version.bnr";
    } else if (crc4 == 0xEE5D && crc1 == 0x4683) {
        // Pokemon Blaze Black (Clean Version)
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Blaze Black (Clean Version).bnr";
    } else if (crc4 == 0xEE5D && crc1 == 0xA251) {
        // Pokemon Blaze Black (Full Version)
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Blaze Black (Full Version).bnr";
    } else if (crc4 == 0x0C88 && crc1 != 0x77F4 && crc1 != 0x9CA8) {
        // Pokemon White Version
        fixedBannerPath = "nitro:/fixedbanners/Pokemon White Version.bnr";
    } else if (crc4 == 0x0C88 && crc1 == 0x77F4) {
        // Pokemon Volt White (Clean Version)
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Volt White (Clean Version).bnr";
    } else if (crc4 == 0x0C88 && crc1 == 0x9CA8) {
        // Pokemon Volt White (Full Version)
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Volt White (Full Version).bnr";
    } else if (crc4 == 0x2CA3) {
        // Pokemon Black Version 2
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Black Version 2.bnr";
    } else if (crc4 == 0x3B18) {
        // Pokemon White Version 2
        fixedBannerPath = "nitro:/fixedbanners/Pokemon White Version 2.bnr";
    }

    if (fixedBannerPath.empty())
        return;

    dbg_printf("!!Fixed banner detected\n");
    FILE *fixedBannerFile = fopen(fixedBannerPath.c_str(), "rb");
    if (fixedBannerFile)
    {
        fread(&*banner, NDS_BANNER_SIZE_DSi, 1, fixedBannerFile);
        dbg_printf("!!Loaded fixed banner.\n");
    }
    fclose(fixedBannerFile);
}
#endif