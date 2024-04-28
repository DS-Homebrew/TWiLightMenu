/*
    gdi.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <cstring>
#include <cstdio>
#include <nds.h>
#include <nds/arm9/cache.h>
#include <nds/system.h>
#include <nds/arm9/background.h>
#include "drawing/gdi.h"
#include "drawing/sprite.h"
#include "tool/memtool.h"
#include "font/fontfactory.h"
#include "tool/dbgtool.h"

#ifdef DEBUG
PrintConsole custom_console;

static void MyInitConsole(u16 *aBufferSub1, u16 *aBufferSub2)
{
    custom_console = *consoleGetDefault();

    custom_console.loadGraphics = false;

    consoleInit(&custom_console, custom_console.bgLayer, BgType_Text4bpp, BgSize_T_256x256, custom_console.mapBase, custom_console.gfxBase, false, false);

    custom_console.fontBgMap = aBufferSub1;
    custom_console.fontBgGfx = aBufferSub2;

    dmaCopy(custom_console.font.gfx, custom_console.fontBgGfx, custom_console.font.numChars * 64 / 2);
    custom_console.fontCurPal = 15 << 12;

    u16 *palette = BG_PALETTE_SUB;
    palette[1 * 16 - 15] = RGB15(0, 0, 0);   //30 normal black
    palette[2 * 16 - 15] = RGB15(15, 0, 0);  //31 normal red
    palette[3 * 16 - 15] = RGB15(0, 15, 0);  //32 normal green
    palette[4 * 16 - 15] = RGB15(15, 15, 0); //33 normal yellow

    palette[5 * 16 - 15] = RGB15(0, 0, 15);   //34 normal blue
    palette[6 * 16 - 15] = RGB15(15, 0, 15);  //35 normal magenta
    palette[7 * 16 - 15] = RGB15(0, 15, 15);  //36 normal cyan
    palette[8 * 16 - 15] = RGB15(24, 24, 24); //37 normal white

    palette[9 * 16 - 15] = RGB15(15, 15, 15); //40 bright black
    palette[10 * 16 - 15] = RGB15(31, 0, 0);  //41 bright red
    palette[11 * 16 - 15] = RGB15(0, 31, 0);  //42 bright green
    palette[12 * 16 - 15] = RGB15(31, 31, 0); //43 bright yellow

    palette[13 * 16 - 15] = RGB15(0, 0, 31);   //44 bright blue
    palette[14 * 16 - 15] = RGB15(31, 0, 31);  //45 bright magenta
    palette[15 * 16 - 15] = RGB15(0, 31, 31);  //46 bright cyan
    palette[16 * 16 - 15] = RGB15(31, 31, 31); //47 & 39 bright white
}
#endif

static inline void dmaCopyWordsGdi(uint8 channel, const void *src, void *dest, uint32 size)
{
    DC_FlushRange(src, size);
    dmaCopyWords(channel, src, dest, size);
    DC_InvalidateRange(dest, size);
}

Gdi::Gdi()
{
    _transColor = 0;
    _mainEngineLayer = MEL_UP;
    _subEngineMode = SEM_TEXT;
    _bufferMain2 = NULL;
    _bufferSub2 = NULL;
#ifdef DEBUG
    _bufferSub3 = NULL;
#endif
    _sprites = NULL;
}

Gdi::~Gdi()
{
    if (NULL != _bufferMain2)
        delete[] _bufferMain2;
    if (NULL != _bufferSub2)
        delete[] _bufferSub2;
#ifdef DEBUG
    if (NULL != _bufferSub3)
        delete[] _bufferSub3;
#endif
    if (NULL != _sprites)
        delete[] _sprites;
}

void Gdi::init()
{
    nocashMessage("ARM9 gdi.cpp init");
    swapLCD();
    nocashMessage("ARM9 gdi.cpp init/swapLCD");

    activeFbMain();
    nocashMessage("ARM9 gdi.cpp init/Main");

    activeFbSub();
    nocashMessage("ARM9 gdi.cpp init/Sub");

    Sprite::sysinit();
    nocashMessage("ARM9 gdi.cpp sysInit");
}

void Gdi::initBg(const std::string &aFileName)
{
    nocashMessage("ARM9 GDI InitBG");
    _sprites = new Sprite[12];
    _background = createBMP15FromFile(aFileName);
    nocashMessage("ARM9 GDI BMP15 Created");
    dbg_printf("Hello %X", _background.buffer());
    if (_background.width() < SCREEN_WIDTH && _background.height() < SCREEN_WIDTH)
    {
        nocashMessage("BG Width too small");
        _background = createBMP15(SCREEN_WIDTH, SCREEN_HEIGHT);
        zeroMemory(_background.buffer(), _background.height() * _background.pitch());
    }

    u32 pitch = _background.pitch() >> 1;
    for (size_t y = 0; y < 3; ++y)
    {
        for (size_t x = 0; x < 4; ++x)
        {
            size_t index = y * 4 + x;

            _sprites[index].init(2 + index);
            _sprites[index].setSize(SS_SIZE_64);
            _sprites[index].setPriority(3);
            _sprites[index].setBufferOffset(32 + index * 64);
            _sprites[index].setPosition(x * 64, y * 64);
            for (size_t k = 0; k < 64; ++k)
            {
                for (size_t l = 0; l < 64; ++l)
                {
                    ((u16 *)_sprites[index].buffer())[k * 64 + l] = ((u16 *)_background.buffer())[(k + y * 64) * pitch + (l + x * 64)];
                }
            }
            _sprites[index].show();
        }
    }
    oamUpdate(&oamMain);
}

void Gdi::swapLCD(void)
{
    lcdSwap();
}

void Gdi::activeFbMain(void)
{
    vramSetBankA(VRAM_A_MAIN_SPRITE_0x06400000);
    vramSetBankB(VRAM_B_MAIN_BG_0x06000000);
    vramSetBankD(VRAM_D_MAIN_BG_0x06020000);

    nocashMessage("ARM9 activeFBMain/vramOK");

    REG_BG2CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY_1;
    REG_BG2PA = 1 << 8;
    REG_BG2PD = 1 << 8;
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2Y = 0;
    REG_BG2X = 0;

    REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(8) | BG_PRIORITY_2;
    REG_BG3PA = 1 << 8;
    REG_BG3PD = 1 << 8;
    REG_BG3PB = 0;
    REG_BG3PC = 0;
    REG_BG3Y = 0;
    REG_BG3X = 0;

    nocashMessage("ARM9 bgSetupOK");

    _bufferMain1 = (u16 *)0x06000000;
    _bufferMain2 = (u16 *)new u32[256 * 192];
    _bufferMain3 = (u16 *)0x06020000;

    setMainEngineLayer(MEL_UP);
    nocashMessage("ARM9 MEL OK");

    nocashMessage("FREE");

    REG_BLDCNT = BLEND_ALPHA | BLEND_DST_BG2 | BLEND_DST_BG3;
    REG_BLDALPHA = (4 << 8) | 7;

    // swiWaitForVBlank(); //remove tearing at bottop screen
    videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE | DISPLAY_SPR_1D_BMP_SIZE_128 | DISPLAY_SPR_1D_BMP);
}

void Gdi::activeFbSub(void)
{
#ifdef DEBUG
    _bufferSub3 = (u16 *)new u32[0x1200];
    MyInitConsole(_bufferSub3 + 0x2000, _bufferSub3);
#endif

    vramSetBankC(VRAM_C_SUB_BG_0x06200000); // 128k

    _subEngineMode = SEM_GRAPHICS;
    //_subEngineMode = SEM_TEXT;

    REG_BG2CNT_SUB = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY_1;
    REG_BG2PA_SUB = 1 << 8;
    REG_BG2PD_SUB = 1 << 8;
    REG_BG2PB_SUB = 0;
    REG_BG2PC_SUB = 0;
    REG_BG2Y_SUB = 0;
    REG_BG2X_SUB = 0;

    _bufferSub1 = (u16 *)0x06200000;
    _bufferSub2 = (u16 *)new u32[256 * 192 / 2];

    //fillMemory( _bufferSub2, 0x18000, 0xfc00fc00 );
    // //fillMemory( _bufferSub1, 0x18000, 0xfc00fc00 );
    // fillMemory(_bufferSub2, 0x18000, 0xffffffff);
    // fillMemory(_bufferSub1, 0x18000, 0xffffffff);

#ifdef DEBUG
    BG_PALETTE_SUB[255] = RGB15(31, 31, 31); //by default font will be rendered with color 255
    REG_BG0CNT_SUB = BG_TILE_BASE(0) | BG_MAP_BASE(8) | BG_PRIORITY_2;
#endif

    // swiWaitForVBlank(); //remove tearing at top screen
    videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE); // | DISPLAY_BG2_ACTIVE );
}

void Gdi::drawLine(s16 x1, s16 y1, s16 x2, s16 y2, GRAPHICS_ENGINE engine)
{
    if ((x1 == x2) && (y1 == y2))
        return;

    if (x1 == x2)
    {
        int ys, ye;
        if (y1 < y2)
        {
            ys = y1;
            ye = y2 - 1;
        }
        else
        {
            ys = y2 + 1;
            ye = y1;
        }
        for (int py = ys; py <= ye; py++)
        {
            drawPixel(x1, py, engine);
        }
        return;
    }

    if (y1 == y2)
    {
        int xs, xe;
        if (x1 < x2)
        {
            xs = x1;
            xe = x2 - 1;
        }
        else
        {
            xs = x2 + 1;
            xe = x1;
        }
        //for (int px=xs;px<=xe;px++) {
        //    drawPixel(px,y1,engine);
        //    //SetPixel(px,y1,Color);
        //}
        if (GE_MAIN == engine)
            fillRect(_penColor, _penColor, xs, y1, xe - xs + 1, 1, engine);
        else
            fillRect(_penColorSub, _penColorSub, xs, y1, xe - xs + 1, 1, engine);
        return;
    }

    if (abs(x2 - x1) > abs(y2 - y1))
    {
        int px = 0;
        float py = 0;
        int xe = x2 - x1;
        float ye = y2 - y1;
        int xv;
        float yv;

        if (0 < xe)
        {
            xv = 1;
        }
        else
        {
            xv = -1;
        }
        yv = ye / abs(xe);

        while (px != xe)
        {
            drawPixel(x1 + px, y1 + (int)py, engine);
            px += xv;
            py += yv;
        }
        return;
    }
    else
    {
        float px = 0;
        int py = 0;
        float xe = x2 - x1;
        int ye = y2 - y1;
        float xv;
        int yv;

        xv = xe / abs(ye);
        if (0 < ye)
        {
            yv = 1;
        }
        else
        {
            yv = -1;
        }

        while (py != ye)
        {
            //if (AALineFlag==false){
            drawPixel(x1 + (int)px, y1 + py, engine);
            //}else{
            //    int Alpha=(int)(px*32);
            //    if (Alpha<0){
            //        while (Alpha<=0) Alpha+=32;
            //    }else{
            //        while (32<=Alpha) Alpha-=32;
            //    }
            //    SetPixelAlpha(x1+(int)px+0,y1+py,Color,32-Alpha);
            //    SetPixelAlpha(x1+(int)px+1,y1+py,Color,Alpha);
            //}
            px += xv;
            py += yv;
        }
        return;
    }
}

void Gdi::frameRect(s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine)
{
    drawLine(x, y, x + w - 1, y, engine);
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, engine);
    drawLine(x + w - 1, y + h - 1, x, y + h - 1, engine);
    drawLine(x, y + h - 1, x, y, engine);
}

void Gdi::frameRect(s16 x, s16 y, u16 w, u16 h, u16 thickness, GRAPHICS_ENGINE engine)
{
    for (size_t ii = 0; ii < thickness; ++ii)
    {
        frameRect(x, y, w, h, engine);
        if (h <= 2 || w <= 2)
            break;
        ++x;
        ++y;
        w -= 2;
        h -= 2;
    }
}

void Gdi::fillRect(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine)
{
    ALIGN(4)
    u16 color[2] = {BIT(15) | color1, BIT(15) | color2};
    u16 *pSrc = (u16 *)color;
    u16 *pDest = NULL;

    if (GE_MAIN == engine)
        pDest = _bufferMain2 + (y << 8) + x + _layerPitch; //_bufferMain2 + y * 256 + x + _layerPitch;
    else
        pDest = _bufferSub2 + (y << 8) + x; //_bufferSub2 + y * 256 + x;

    bool destAligned = !(x & 1);

    u16 destInc = 256 - w;
    u16 halfWidth = w >> 1;
    u16 remain = w & 1;

    if (destAligned)
        for (u32 i = 0; i < h; ++i)
        {
            swiFastCopy(pSrc, pDest, COPY_MODE_WORD | COPY_MODE_FILL | halfWidth);
            pDest += halfWidth << 1;
            if (remain)
                *pDest++ = *pSrc;
            pDest += destInc;
        }
    else
        for (u32 i = 0; i < h; ++i)
        {
            for (u32 j = 0; j < w; ++j)
            {
                *pDest++ = pSrc[j & 1];
            }
            pDest += destInc;
        }
}

void Gdi::fillRectBlend(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine, u16 opacity)
{
    if (opacity == 0)
        return;
    if (opacity == 100)
    {
        fillRect(color1, color2, x, y, w, h, engine);
        return;
    }
    u16 *pSrc = ((GE_MAIN == engine) ? (u16 *)_background.buffer() : _bufferSub2) + (y << 8) + x;
    u16 *pDest = ((GE_MAIN == engine) ? (_bufferMain2 + _layerPitch) : _bufferSub2) + (y << 8) + x;
    u32 alpha = (opacity * 32) / 100;
    u32 destInc = 256 - w;

    for (u32 ii = 0; ii < h; ++ii)
    {
        for (u32 jj = 0; jj < w; ++jj)
        {
            u32 original = *pSrc++ & 0x7fff;
            u32 color = (jj & 1) ? color2 : color1;
            u32 rb = ((color & 0x7c1f) * alpha + (original & 0x7c1f) * (32 - alpha)) & 0xf83e0;
            u32 g = ((color & 0x3e0) * alpha + (original & 0x3e0) * (32 - alpha)) & 0x7c00;
            *pDest++ = ((rb | g) >> 5) | BIT(15);
        }
        pDest += destInc;
        pSrc += destInc;
    }
}

void Gdi::bitBlt(const void *src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine)
{
    if (destW <= 0)
        return;

    u16 *pSrc = (u16 *)src;
    u16 *pDest = NULL;

    if (GE_MAIN == engine)
        pDest = _bufferMain2 + (destY)*256 + destX + _layerPitch;
    else
        pDest = _bufferSub2 + (destY)*256 + destX;

    bool destAligned = !(destX & 1);

    if (destW > srcW)
        destW = srcW;
    if (destH > srcH)
        destH = srcH;

    u16 srcInc = srcW - destW;
    u16 destInc = 256 - destW;
    u16 destHalfWidth = destW >> 1;
    u16 lineSize = destW << 1;
    u16 remain = destW & 1;

    if (destAligned)
    {
        for (u32 i = 0; i < destH; ++i)
        {
            dmaCopyWordsGdi(3, pSrc, pDest, lineSize);
            pDest += destHalfWidth << 1;
            pSrc += destHalfWidth << 1;
            if (remain)
                *pDest++ = *pSrc++;
            pDest += destInc;
            pSrc += srcInc;
        }
    }
}

void Gdi::bitBlt(const void *src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine)
{
    //dbg_printf("x %d y %d w %d h %d\n", destX, destY, destW, destH );
    u16 *pSrc = (u16 *)src;
    u16 *pDest = NULL;

    if (GE_MAIN == engine)
        pDest = _bufferMain2 + (destY)*256 + destX + _layerPitch;
    else
        pDest = _bufferSub2 + (destY)*256 + destX;

    u16 pitchPixel = (destW + (destW & 1));
    u16 destInc = 256 - pitchPixel;
    u16 halfPitch = pitchPixel >> 1;
    u16 remain = pitchPixel & 1;

    for (u16 i = 0; i < destH; ++i)
    {
        swiFastCopy(pSrc, pDest, COPY_MODE_WORD | COPY_MODE_COPY | halfPitch);
        pDest += halfPitch << 1;
        pSrc += halfPitch << 1;
        if (remain)
            *pDest++ = *pSrc++;
        pDest += destInc;
    }
}

void Gdi::maskBlt(const void *src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine)
{
    //dbg_printf("x %d y %d w %d h %d\n", destX, destY, destW, destH );
    u16 *pSrc = (u16 *)src;
    u16 *pDest = NULL;
    bool destAligned = !(destX & 1);

    if (GE_MAIN == engine)
        pDest = _bufferMain2 + (destY)*256 + destX + _layerPitch;
    else
        pDest = _bufferSub2 + (destY)*256 + destX;

    u16 pitch = (destW + (destW & 1));
    u16 destInc = 256 - pitch;
    u16 halfPitch = pitch >> 1;

    if (destAligned)
        for (u32 i = 0; i < destH; ++i)
        {
            for (u32 j = 0; j < halfPitch; ++j)
            {
                if (((*(u32 *)pSrc) & 0x80008000) == 0x80008000)
                {
                    *(u32 *)pDest = *(u32 *)pSrc;
                    pSrc += 2;
                    pDest += 2;
                }
                else
                {
                    if (*pSrc & 0x8000)
                        *pDest = *pSrc;
                    pSrc++;
                    pDest++;
                    if (*pSrc & 0x8000)
                        *pDest = *pSrc;
                    pSrc++;
                    pDest++;
                }
            }
            pDest += destInc;
        }
    else
        for (u16 i = 0; i < destH; ++i)
        {
            for (u16 j = 0; j < pitch; ++j)
            {
                if (*pSrc & 0x8000)
                    *pDest = *pSrc;
                pDest++;
                pSrc++;
            }
            pDest += destInc;
        }
}

void Gdi::maskBlt(const void *src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine)
{
    if (destW <= 0)
        return;

    u16 *pSrc = (u16 *)src;
    u16 *pDest = NULL;

    if (GE_MAIN == engine)
        pDest = _bufferMain2 + (destY)*256 + destX + _layerPitch;
    else
        pDest = _bufferSub2 + (destY)*256 + destX;

    bool destAligned = !(destX & 1);

    if (destW > srcW)
        destW = srcW;
    if (destH > srcH)
        destH = srcH;

    u16 srcInc = srcW - destW;
    u16 destInc = 256 - destW;
    u16 destHalfWidth = destW >> 1;
    u16 pitch = (destW + (destW & 1));
    u16 remain = destW & 1;

    if (destAligned)
    {
        for (u32 i = 0; i < destH; ++i)
        {
            for (u32 j = 0; j < destHalfWidth; ++j)
            {
                if (((*(u32 *)pSrc) & 0x80008000) == 0x80008000)
                {
                    *(u32 *)pDest = *(u32 *)pSrc;
                    pSrc += 2;
                    pDest += 2;
                }
                else
                {
                    if (*pSrc & 0x8000)
                        *pDest = *pSrc;
                    pSrc++;
                    pDest++;
                    if (*pSrc & 0x8000)
                        *pDest = *pSrc;
                    pSrc++;
                    pDest++;
                }
            }
            if (remain)
                *pDest++ = *pSrc++;
            pDest += destInc;
            pSrc += srcInc;
        }
    }
    else
        for (u16 i = 0; i < destH; ++i)
        {
            for (u16 j = 0; j < pitch; ++j)
            {
                if (*pSrc & 0x8000)
                    *pDest = *pSrc;
                pDest++;
                pSrc++;
            }
            pDest += destInc;
            pSrc += srcInc;
        }
}

void Gdi::textOutRect(s16 x, s16 y, u16 w, u16 h, const char *text, GRAPHICS_ENGINE engine)
{
    const s16 originX = x, limitY = y + h - SYSTEM_FONT_HEIGHT;
    while (*text)
    {
        if ('\r' == *text || '\n' == *text)
        {
            y += SYSTEM_FONT_HEIGHT; //FIXME
            x = originX;
            ++text;
            if (y > limitY)
                break;
        }
        else
        {
            u32 ww, add;
            font().Info(text, &ww, &add);
            if (x + (s16)ww < originX + w)
            {
                font().Draw((GE_MAIN == engine) ? (_bufferMain2 + _layerPitch) : _bufferSub2, x, y, (const u8 *)text, (GE_MAIN == engine) ? _penColor : _penColorSub);
            }
            text += add;
            x += ww;
        }
    }
}

void Gdi::present(GRAPHICS_ENGINE engine)
{
    if (GE_MAIN == engine)
    {
        // u16 * temp = _bufferMain1;
        // _bufferMain1 = _bufferMain2;
        // _bufferMain2 = temp;
        // REG_BG2CNT ^= BG_BMP_BASE( 128 / 16 );

        dmaCopyWordsGdi(3, _bufferMain2 + _layerPitch,
                        _bufferMain1 + (_mainEngineLayer << 16), 256 * 192 * 2);

        fillMemory((void *)(_bufferMain2 + _layerPitch), 256 * 192 * 2, 0);
        oamUpdate(&oamMain);
    }
    else if (GE_SUB == engine)
    {
        if (SEM_GRAPHICS == _subEngineMode)
            dmaCopyWordsGdi(3, (void *)_bufferSub2, (void *)_bufferSub1, 256 * 192 * 2);
        //else if ( SEM_TEXT == _subEngineMode )
        //    dmaCopyWords( 3, (void *)_bufferSub3, (void *)_bufferSub1, 32768 );
        fillMemory((void *)_bufferSub2, 0x18000, 0xffffffff);
    }
    //dbg_printf( "\x1b[0;20%f\n", updateTimer() );
}

//special version for window switching
void Gdi::present(void)
{
    swiWaitForVBlank();
    dmaCopyWordsGdi(3, _bufferMain2, _bufferMain1, 256 * 192 * 2);
    dmaCopyWordsGdi(3, _bufferMain2 + (256 * 192), _bufferMain1 + (1 << 16), 256 * 192 * 2);
    fillMemory((void *)_bufferMain2, 256 * 192 * 4, 0);
}

#ifdef DEBUG
void Gdi::switchSubEngineMode()
{
    switch (_subEngineMode)
    {
    case SEM_GRAPHICS:
        videoSetModeSub(MODE_5_2D | DISPLAY_BG0_ACTIVE);
        custom_console.fontBgMap = (u16 *)0x6204000;
        custom_console.fontBgGfx = (u16 *)0x6200000;
        dmaCopyWordsGdi(3, (void *)_bufferSub3, (void *)_bufferSub1, 0x4800);
        break;
    case SEM_TEXT:
        videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE);
        custom_console.fontBgMap = _bufferSub3 + 0x2000;
        custom_console.fontBgGfx = _bufferSub3;
        dmaCopyWordsGdi(3, (void *)_bufferSub1, (void *)_bufferSub3, 0x4800);
        break;
    };
    _subEngineMode = (SUB_ENGINE_MODE)(_subEngineMode ^ 1);
}
#endif
