/*
    irqs.cpp
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

#include "irqs.h"
#include "drawing/gdi.h"
#include "time/timer.h"

// #include "userinput.h"
// #include "windowmanager.h"
#include "windows/diskicon.h"
#include "windows/calendarwnd.h"
#include "windows/calendar.h"
#include "windows/bigclock.h"
#include "windows/userwnd.h"
#include "ui/animation.h"

// #include "time/timer.h"
// #include "files.h"
// #include "userwnd.h"

bool cIRQ::_vblankStarted(false);

void cIRQ::init()
{
    irqSet(IRQ_VBLANK, vBlank);
    irqSet(IRQ_CARD_LINE, cardMC);
}

void cIRQ::cardMC()
{
    dbg_printf("cardMC\n");
    diskIcon().blink();
    REG_IF &= ~IRQ_CARD_LINE;
}

void cIRQ::vblankStart()
{
    _vblankStarted = true;
}

void cIRQ::vblankStop()
{
    _vblankStarted = false;
}

void cIRQ::vBlank()
{
    if (!_vblankStarted)
        return;

    // // get inputs when file copying because the main route
    // // can't do any thing at that time
    // if( true == copyingFile) {
    //     if( false == stopCopying ) {
    //         INPUT & input = updateInput();
    //         if( (input.keysDown & KEY_B) ) {
    //             stopCopying = true;
    //         }
    //     }
    // }

    timer().updateTimer();

    static u32 vBlankCounter = 0;

    if (vBlankCounter++ > 30)
    {
        vBlankCounter = 0;
        bigClock().blinkColon();
        calendarWnd().draw();
        calendar().draw();
        bigClock().draw();
        userWindow().draw();
#if 0
        char fpsText[16];
        sprintf( fpsText, "fps %.2f\n", timer().getFps() );
        gdi().setPenColor( 1, GE_SUB );
        gdi().textOut( 40, 178, fpsText, GE_SUB );
#endif

        gdi().present(GE_SUB);
    }

    animationManager().update();

    if( REG_ROMCTRL & CARD_BUSY )
        diskIcon().turnOn();
    else
        diskIcon().turnOff();
}
