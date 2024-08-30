/*---------------------------------------------------------------------------------

	Copyright (C) 2005
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)
		Mike Parks (BigRedPimp)
	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.
	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:
 
  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.
	
---------------------------------------------------------------------------------*/
#include <nds/ndstypes.h>
#include <nds/memory.h>
#include <nds/arm9/dldi.h>
#include "common/my_rumble.h"

static MY_RUMBLE_TYPE rumbleType;

//---------------------------------------------------------------------------------
bool my_isRumbleInserted(void) {
//---------------------------------------------------------------------------------
	//sysSetCartOwner(BUS_OWNER_ARM9);
	// First, make sure DLDI is Slot-1
	if ((io_dldi_data->ioInterface.features & FEATURE_SLOT_GBA) || *(u16*)0x020000C0 != 0 || *(vu16*)0x08240000 == 1 || GBA_BUS[0] == 0xFFFE || GBA_BUS[1] == 0xFFFF) {
		return false;
	}
	// Then, check for 0x96 to see if it's a GBA game or flashcart
	if (GBA_HEADER.is96h == 0x96) {
		rumbleType = MY_WARIOWARE;
		WARIOWARE_ENABLE = 8;
		return true;
	} else {
		rumbleType = MY_RUMBLE;

		// Check for DS Phat or Lite Rumble Pak
		for (int i = 0; i < 0xFFF; i++) {
			if (GBA_BUS[i] != 0xFFFD && GBA_BUS[i] != 0xFDFF) {
				return true;
			}
		}
	}
	return false;
}
//---------------------------------------------------------------------------------
void my_setRumble(bool position) {
//---------------------------------------------------------------------------------
	if (rumbleType == MY_WARIOWARE) {
		WARIOWARE_PAK = (position ? 8 : 0); 
	} else {
		RUMBLE_PAK = (position ? 2 : 0);
	}
}
