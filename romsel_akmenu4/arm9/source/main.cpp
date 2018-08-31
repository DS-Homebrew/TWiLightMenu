/*-----------------------------------------------------------------
 Copyright (C) 2005 - 2013
	Michael "Chishm" Chisholm
	Dave "WinterMute" Murphy
	Claudio "sverx"

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

------------------------------------------------------------------*/
#include <nds.h>
#include <maxmod9.h>

//-- AK Start -----------
#include "drawing/gdi.h"
#include "systemfilenames.h"
#include "time/timer.h"
#include "font/fontfactory.h"
#include "irqs.h"
#include "ui/ui.h"
#include "userinput.h"
#include "language.h"
#include "globalsettings.h"
#include "windows/calendar.h"
#include "windows/calendarwnd.h"
#include "windows/bigclock.h"
#include "windows/diskicon.h"
#include "windows/userwnd.h"
#include "ui/progresswnd.h"
#include "windows/mainwnd.h"
#include "bootstrap_support/systemdetails.h"
// -- AK End ------------

#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>
#include <gl2d.h>

#include "sr_data_srllastran.h"			 // For rebooting into the game (NTR-mode touch screen)
#include "sr_data_srllastran_twltouch.h" // For rebooting into the game (TWL-mode touch screen)

using namespace akui;

//---------------------------------------------------------------------------------
void stop(void)
{
	//---------------------------------------------------------------------------------
	while (1)
	{
		swiWaitForVBlank();
	}
}

char filePath[PATH_MAX];

//---------------------------------------------------------------------------------
void doPause()
{
	//---------------------------------------------------------------------------------
	// iprintf("Press start...\n");
	// printSmall(false, x, y, "Press start...");
	while (1)
	{
		scanKeys();
		if (keysDown() & KEY_START)
			break;
		swiWaitForVBlank();
	}
	scanKeys();
}

// Ported from PAlib (obsolete)
void SetBrightness(u8 screen, s8 bright)
{
	u16 mode = 1 << 14;

	if (bright < 0)
	{
		mode = 2 << 14;
		bright = -bright;
	}
	if (bright > 31)
		bright = 31;
	*(u16 *)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

int main(int argc, char **argv)
{
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	SetBrightness(0, 0);
	SetBrightness(1, 0);

	// consoleDemoInit();
	// printf("Ok...");
	// stop();

	defaultExceptionHandler();

	bool fatInitOk = fatInitDefault();

	sys();

	irq().init();

	// init graphics
	gdi().init();
	gdi().initBg(SFN_LOWER_SCREEN_BG);

	windowManager();

	// init basic system
	sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);

	// init tick timer/fps counter
	timer().initTimer();

	initInput();

	//turn led on
	ledBlink(PM_LED_ON);

	// Prevent the black screen from showing
	//setBackdropColor(RGB15(31, 31, 31));
	//setBackdropColorSub(RGB15(31,31,31));

#ifdef DEBUG
	gdi().switchSubEngineMode();
#endif
	dbg_printf("GDI Init!");

	if (!fatInitOk)
	{
		consoleDemoInit();
		printf("Failed to Init FAT");
		stop();
	}
	// setting scripts
	gs().loadSettings();

	// init unicode
	//if( initUnicode() )
	//    _FAT_unicode_init( unicodeL2UTable, unicodeU2LTable, unicodeAnkTable );
	cwl();

	lang(); // load language file
	gs().language = lang().GetInt("font", "language", gs().language);

	fontFactory().makeFont(); // load font file
	uiSettings().loadSettings();

	diskIcon().loadAppearance(SFN_CARD_ICON_BLUE);
	diskIcon().show();

	timer().updateFps();

	MainWnd *wnd = new MainWnd(0, 0, 256, 192, NULL, "main window");
	wnd->init();

	progressWnd().init();

	//---- Top Screen ---
	calendarWnd().init();
	calendarWnd().draw();
	calendar().init();
	calendar().draw();
	bigClock().init();
	bigClock().draw();

	userWindow().draw();
	windowManager().update();

	//---- END Top Screen--


	gdi().present(GE_MAIN);
	gdi().present(GE_SUB);

	//if (!wnd->_mainList->enterDir(SPATH_ROOT != lastDirectory ? lastDirectory : gs().startupFolder))
	wnd->_mainList->enterDir(SPATH_ROOT);

	irq().vblankStart();

	while (1)
	{
		timer().updateFps();
		INPUT &inputs = updateInput();
		processInput(inputs);
		swiWaitForVBlank();
		windowManager().update();
		gdi().present(GE_MAIN);
	}
	return 0;
}