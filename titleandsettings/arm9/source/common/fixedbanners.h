#include "ndsheader.h"
#ifndef __FIXED_BANNERS_H__
#define __FIXED_BANNERS_H__
#define PACK_CRC(a, b, c, d) ((unsigned long long)a << 48 | (unsigned long long)b << 32 | (unsigned long long)c << 16 | (unsigned long long)d)

inline void fixBanner(sNDSBannerExt *banner)
{
    // Check for fixed banner here.
    u64 crc = PACK_CRC(banner->crc[0], banner->crc[1], banner->crc[2], banner->crc[3]);
    std::string fixedBannerPath;

    switch (crc)
    {
    case 0xECF9D18FE22AD8F4:
        // Fire Emblem - Heroes of Light and Shadow (English Translation)
        fixedBannerPath = "nitro:/fixedbanners/Fire Emblem - Heroes of Light and Shadow (J) (Eng).bnr";
        break;
    case 0x4A1940AD5641EE5D:
        // Pokemon Black Version
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Black Version.bnr";
        break;
    case 0x468340AD5641EE5D:
        // Pokemon Blaze Black (Clean Version)
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Blaze Black (Clean Version).bnr";
        break;
    case 0xA25140AD5641EE5D:
        // Pokemon Blaze Black (Full Version)
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Blaze Black (Full Version).bnr";
        break;
    case 0xE2495C94BF180C88:
        // Pokemon White Version
        fixedBannerPath = "nitro:/fixedbanners/Pokemon White Version.bnr";
        break;
    case 0x77F45C94BF180C88:
        // Pokemon Volt White (Clean Version)
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Volt White (Clean Version).bnr";
        break;
    case 0x9CA85C94BF180C88:
        // Pokemon Volt White (Full Version)
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Volt White (Full Version).bnr";
        break;
    case 0xF996D784A2572CA3:
        // Pokemon Black Version 2
        fixedBannerPath = "nitro:/fixedbanners/Pokemon Black Version 2.bnr";
        break;
    case 0xA487F58CAF9E3B18:
        // Pokemon White Version 2
        fixedBannerPath = "nitro:/fixedbanners/Pokemon White Version 2.bnr";
        break;
    default:
        break;
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