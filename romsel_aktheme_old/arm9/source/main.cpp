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
#include "windows/calendar.h"
#include "windows/calendar_2.h"
#include "windows/calendarwnd.h"
#include "windows/bigclock.h"
#include "windows/smallclock.h"
#include "windows/userwnd.h"
#include "windows/batteryicon.h"
#include "windows/volumeicon.h"
#include "ui/progresswnd.h"
#include "windows/mainwnd.h"
#include "common/systemdetails.h"
#include "common/dsimenusettings.h"
#include "common/flashcard.h"
#include "common/filecopy.h"

// -- AK End ------------

#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>

#include <string.h>
#include <unistd.h>

bool useTwlCfg = false;

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
	*(vu16 *)(0x0400006C + (0x1000 * screen)) = bright + mode;
}

bool extention(const std::string& filename, const char* ext) {
	if (strcasecmp(filename.c_str() + filename.size() - strlen(ext), ext)) {
		return false;
	} else {
		return true;
	}
}

int main(int argc, char **argv)
{
	useTwlCfg = (REG_SCFG_EXT!=0 && (*(u8*)0x02000400 != 0) && (*(u8*)0x02000401 == 0) && (*(u8*)0x02000402 == 0) && (*(u8*)0x02000404 == 0) && (*(u8*)0x02000448 != 0));

	SetBrightness(0, 0);
	SetBrightness(1, 0);

	// consoleDemoInit();
	// printf("Ok...");
	// stop();
	nocashMessage("begin");
	defaultExceptionHandler();
	nocashMessage("except init");
	sys().initFilesystem();
	nocashMessage("fs init");
	sys().initArm7RegStatuses();
	nocashMessage("arm7 init");
	ms().loadSettings();
	nocashMessage("settings init");

	// init basic system
	sysSetBusOwners(BUS_OWNER_ARM9, BUS_OWNER_ARM9);


	sfn().initFilenames();
	nocashMessage("sfn init");
	irq().init();

	gdi().init();
	// init graphics
	gdi().initBg(SFN_LOWER_SCREEN_BG);
	windowManager();


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

	if (!sys().fatInitOk())
	{
		consoleDemoInit();
		printf("Failed to Init FAT");
		stop();
	}
	dbg_printf("UISETTINGS: %s\n", SFN_BUTTON2);
	//stop();
	// if (!nitroFSInitOk)
	// {
	// 	consoleDemoInit();
	// 	printf("Failed to init NitroFS");
	// 	stop();
	// }


	
	// init unicode
	//if ( initUnicode() )
	//    _FAT_unicode_init( unicodeL2UTable, unicodeU2LTable, unicodeAnkTable );
	cwl();

	lang(); // load language file

	fontFactory().makeFont(); // load font file
	uiSettings().loadSettings();

	// diskIcon().loadAppearance(SFN_CARD_ICON_BLUE);
	// diskIcon().show();

	batteryIcon().draw();

	volumeIcon().draw();

	timer().updateFps();

	MainWnd *wnd = new MainWnd(0, 0, 256, 192, NULL, "main window");
	wnd->init();

	progressWnd().init();

	//---- Top Screen ---
	calendarWnd().init();
	calendarWnd().draw();
	calendar().init();
	calendar().draw();
	calendar_2().init();
	calendar_2().draw();
	bigClock().init();
	bigClock().draw();
	smallClock().init();
	smallClock().draw();

	userWindow().draw();
	windowManager().update();

	//---- END Top Screen--


	gdi().present(GE_MAIN);
	gdi().present(GE_SUB);

	if ((ms().consoleModel < 2 && ms().previousUsedDevice && bothSDandFlashcard() && ms().launchType[ms().previousUsedDevice] == TWLSettings::TLaunchType::EDSiWareLaunch &&
	     access(ms().dsiWarePubPath.c_str(), F_OK) == 0 && !ms().dsiWareBooter && extention(ms().dsiWarePubPath.c_str(), ".pub")) ||
	    (ms().consoleModel < 2 && ms().previousUsedDevice && bothSDandFlashcard() && ms().launchType[ms().previousUsedDevice] == TWLSettings::TLaunchType::EDSiWareLaunch &&
	     access(ms().dsiWarePrvPath.c_str(), F_OK) == 0 && !ms().dsiWareBooter && extention(ms().dsiWarePrvPath.c_str(), ".prv")))
	{
		if (access(ms().dsiWarePubPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.pub", ms().dsiWarePubPath.c_str());
		}
		if (access(ms().dsiWarePrvPath.c_str(), F_OK) == 0) {
			fcopy("sd:/_nds/TWiLightMenu/tempDSiWare.prv", ms().dsiWarePrvPath.c_str());
		}
	}

	//if (!wnd->_mainList->enterDir(SPATH_ROOT != lastDirectory ? lastDirectory : gs().startupFolder))
	wnd->_mainList->enterDir(ms().romfolder[ms().secondaryDevice]);

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