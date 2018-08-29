#include <nds.h>

#ifndef __CARD_LAUNCH__
#define __CARD_LAUNCH__
void cardLaunch()
{
    *(u32 *)(0x02000300) = 0x434E4C54; // Set "CNLT" warmboot flag
    *(u16 *)(0x02000304) = 0x1801;
    *(u32 *)(0x02000308) = 0x43415254; // "CART"
    *(u32 *)(0x0200030C) = 0x00000000;
    *(u32 *)(0x02000310) = 0x43415254; // "CART"
    *(u32 *)(0x02000314) = 0x00000000;
    *(u32 *)(0x02000318) = 0x00000013;
    *(u32 *)(0x0200031C) = 0x00000000;
    while (*(u16 *)(0x02000306) == 0x0000)
    { // Keep running, so that CRC16 isn't 0
        *(u16 *)(0x02000306) = swiCRC16(0xFFFF, (void *)0x02000308, 0x18);
    }

    fifoSendValue32(FIFO_USER_02, 1); // Reboot into DSiWare title, booted via Launcher
}

void dsiSysMenuLaunch()
{
    *(u32 *)(0x02000300) = 0x434E4C54; // Set "CNLT" warmboot flag
    *(u16 *)(0x02000304) = 0x1801;
    *(u32 *)(0x02000310) = 0x4D454E55; // "MENU"
    fifoSendValue32(FIFO_USER_02, 1);  // ReturntoDSiMenu
}

void dsiLaunchSystemSettings()
{
    char tmdpath[256];
    u8 titleID[4];
    for (u8 i = 0x41; i <= 0x5A; i++)
    {
        snprintf(tmdpath, sizeof(tmdpath), "sd:/title/00030015/484e42%x/content/title.tmd", i);
        if (!access(tmdpath, F_OK))
        {
            titleID[0] = i;
            titleID[1] = 0x42;
            titleID[2] = 0x4e;
            titleID[3] = 0x48;
            break;
        }
    }

    *(u32 *)(0x02000300) = 0x434E4C54; // Set "CNLT" warmboot flag
    *(u16 *)(0x02000304) = 0x1801;
    *(u8 *)(0x02000308) = titleID[0];
    *(u8 *)(0x02000309) = titleID[1];
    *(u8 *)(0x0200030A) = titleID[2];
    *(u8 *)(0x0200030B) = titleID[3];
    *(u32 *)(0x0200030C) = 0x00030015;
    *(u8 *)(0x02000310) = titleID[0];
    *(u8 *)(0x02000311) = titleID[1];
    *(u8 *)(0x02000312) = titleID[2];
    *(u8 *)(0x02000313) = titleID[3];
    *(u32 *)(0x02000314) = 0x00030015;
    *(u32 *)(0x02000318) = 0x00000017;
    *(u32 *)(0x0200031C) = 0x00000000;
    while (*(u16 *)(0x02000306) == 0x0000)
    { // Keep running, so that CRC16 isn't 0
        *(u16 *)(0x02000306) = swiCRC16(0xFFFF, (void *)0x02000308, 0x18);
    }

    fifoSendValue32(FIFO_USER_08, 1); // Reboot into System Settings
}

#endif