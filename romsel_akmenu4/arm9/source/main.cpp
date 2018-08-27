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
#include "gdi.h"
#include "systemfilenames.h"
#include "timer.h"
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

// -- AK End ------------

#include <stdio.h>
#include <fat.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>
#include <gl2d.h>

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

int main(int argc, char **argv)
{
	extern u64 *fake_heap_end;
	*fake_heap_end = 0;

	nocashMessage("ARM9 main.cpp");
	defaultExceptionHandler();

	irq().init();
	nocashMessage("ARM9 main.cpp IRQ Init");

	windowManager();
	nocashMessage("ARM9 main.cpp WindowManager Init");

	// init basic system
	sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);

	// init tick timer/fps counter
	timer().initTimer();
	nocashMessage("Timer Init!");

	initInput();
	nocashMessage("ARM9 main.cpp Input Init");

	// init graphics
	gdi().init();
#ifdef DEBUG
	//gdi().switchSubEngineMode();
#endif
	nocashMessage("GDI Init!");

	bool fatInitOk = fatInitDefault();
	if (!fatInitOk)
	{
		consoleDemoInit();
		printf("Failed to Init FAT");
		stop();
	}
	cwl();

	lang(); // load language file
	gs().language = lang().GetInt("font", "language", gs().language);

	fontFactory().makeFont(); // load font file
	uiSettings().loadSettings();

	diskIcon().loadAppearance(SFN_CARD_ICON_BLUE);
	diskIcon().show();

	windowManager().update();
	timer().updateFps();

	//---- Top Screen ---
	calendarWnd().init();
	calendarWnd().draw();
	calendar().init();
	calendar().draw();
	bigClock().init();
	bigClock().draw();
	//---- END Top Screen--

	gdi().initBg(SFN_LOWER_SCREEN_BG);
	gdi().present(GE_MAIN);
	gdi().present(GE_SUB);

	irq().vblankStart();

	// fifoWaitValue32(FIFO_USER_06);
	// if (fifoGetValue32(FIFO_USER_03) == 0)
	// 	arm7SCFGLocked = true; // If DSiMenu++ is being ran from DSiWarehax or flashcard, then arm7 SCFG is locked.
	// u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
	// if (arm7_SNDEXCNT != 0)
	// 	isRegularDS = false; // If sound frequency setting is found, then the console is not a DS Phat/Lite
	// fifoSendValue32(FIFO_USER_07, 0);

	// gdi().initBg(SFN_LOWER_SCREEN_BG);
	// nocashMessage("LOWER_BG");
	// nocashMessage(SFN_LOWER_SCREEN_BG);
	stop();
}