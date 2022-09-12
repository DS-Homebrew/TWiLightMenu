/*
    gdi.h
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

#ifndef _GDI_H_
#define _GDI_H_

#include <nds.h>
#include <vector>
#include "common/singleton.h"
#include "drawing/gdi.h"
#include "drawing/bmp15.h"
#include "tool/dbgtool.h"

#define SYSTEM_FONT_HEIGHT 12
#define COLOR u16

enum GRAPHICS_ENGINE
{
    GE_MAIN = 0,
    GE_SUB = 1
};

enum MAIN_ENGINE_LAYER
{
    MEL_UP = 0,
    MEL_DOWN = 1
};

enum SUB_ENGINE_MODE
{
    SEM_TEXT = 0,
    SEM_GRAPHICS = 1
};

class Sprite;

class Gdi
{
  public:
    Gdi();

    virtual ~Gdi();

  public:
    void init();

    void initBg(const std::string &aFileName);

    void drawPixel(u8 x, u8 y, GRAPHICS_ENGINE engine)
    {
        //if ( y > (u8)SCREEN_HEIGHT - 1 ) y = (u8)SCREEN_WIDTH - 1;
        if (GE_MAIN == engine)
            *(_bufferMain2 + ((u32)y << 8) + x + _layerPitch) = _penColor; //_bufferMain2[y * SCREEN_WIDTH + x] = _penColor;
        else
            _bufferSub2[((u32)y << 8) + x] = _penColor; //_bufferSub2[y * SCREEN_WIDTH + x] = _penColor;
    }

    void drawLine(s16 x1, s16 y1, s16 x2, s16 y2, GRAPHICS_ENGINE engine);

    void frameRect(s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine);

    void frameRect(s16 x, s16 y, u16 w, u16 h, u16 thickness, GRAPHICS_ENGINE engine);

    void fillRect(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine);

    void fillRectBlend(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine, u16 opacity);

    void maskBlt(const void *src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine);

    void maskBlt(const void *src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine);

    void bitBlt(const void *src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine);

    void bitBlt(const void *src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine);

    u16 getPenColor(GRAPHICS_ENGINE engine)
    {
        if (GE_MAIN == engine)
            return _penColor & ~BIT(15);
        else
            return _penColorSub & ~BIT(15);
    }

    void setPenColor(u16 color, GRAPHICS_ENGINE engine)
    {
        if (GE_MAIN == engine)
            _penColor = color | BIT(15);
        else
            _penColorSub = color | BIT(15);
    }

    void setTransColor(u16 color) { _transColor = color | BIT(15); }

    void textOutRect(s16 x, s16 y, u16 w, u16 h, const char *text, GRAPHICS_ENGINE engine);

    inline void textOut(s16 x, s16 y, const char *text, GRAPHICS_ENGINE engine)
    {
        textOutRect(x, y, 256, 192, text, engine);
    }

    void setMainEngineLayer(MAIN_ENGINE_LAYER layer)
    {
        _mainEngineLayer = layer;
        _layerPitch = layer * 256 * 192;
    }

    void present(GRAPHICS_ENGINE engine);

    void present(void);

#ifdef DEBUG
    void switchSubEngineMode();
#endif

  protected:
    void swapLCD(void);
    void activeFbMain(void); // fb = frame buffer
    void activeFbSub(void);

  private:
    std::vector<u16> _penColorStack;
    u16 _penColor;
    u16 _penColorSub;
    u16 _transColor;
    u16 *_bufferMain1;
    u16 *_bufferMain2;
    u16 *_bufferMain3;
    MAIN_ENGINE_LAYER _mainEngineLayer;
    SUB_ENGINE_MODE _subEngineMode;
    u32 _layerPitch;
    u16 *_bufferSub1;
    u16 *_bufferSub2;
#ifdef DEBUG
    u16 *_bufferSub3;
#endif
    Sprite *_sprites;
    BMP15 _background;
};

typedef singleton<Gdi> Gdi_s;
inline Gdi &gdi() { return Gdi_s::instance(); }

#endif //_GDI_H_
