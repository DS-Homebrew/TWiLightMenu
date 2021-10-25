/*
 disc.c
 Interface to the low level disc functions. Used by the higher level
 file system code.
 Based on code originally written by MightyMax

 Copyright (c) 2006 Michael "Chishm" Chisholm
	
 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <nds/arm9/dldi.h>
#include <nds/system.h>

#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <sys/fcntl.h>

const DLDI_INTERFACE* io_dldi_data = (DLDI_INTERFACE*)0x03038000;

void dldiFixDriverAddresses (DLDI_INTERFACE* io) {
	u32 offset;
	u8** address;
	u8* oldStart;
	u8* oldEnd;
	
	offset = (char*)io - (char*)(io->dldiStart);

	oldStart = io->dldiStart;
	oldEnd = io->dldiEnd;
	
	// Correct all pointers to the offsets from the location of this interface
	io->dldiStart 		= (char*)io->dldiStart + offset;
	io->dldiEnd 		= (char*)io->dldiEnd + offset;
	io->interworkStart 	= (char*)io->interworkStart + offset;
	io->interworkEnd 	= (char*)io->interworkEnd + offset;
	io->gotStart 		= (char*)io->gotStart + offset;
	io->gotEnd 			= (char*)io->gotEnd + offset;
	io->bssStart 		= (char*)io->bssStart + offset;
	io->bssEnd 			= (char*)io->bssEnd + offset;
	
	io->ioInterface.startup 		= (FN_MEDIUM_STARTUP)		((intptr_t)io->ioInterface.startup + offset);
	io->ioInterface.isInserted 		= (FN_MEDIUM_ISINSERTED)	((intptr_t)io->ioInterface.isInserted + offset);
	io->ioInterface.readSectors 	= (FN_MEDIUM_READSECTORS)	((intptr_t)io->ioInterface.readSectors + offset);
	io->ioInterface.writeSectors	= (FN_MEDIUM_WRITESECTORS)	((intptr_t)io->ioInterface.writeSectors + offset);
	io->ioInterface.clearStatus 	= (FN_MEDIUM_CLEARSTATUS)	((intptr_t)io->ioInterface.clearStatus + offset);
	io->ioInterface.shutdown 		= (FN_MEDIUM_SHUTDOWN)		((intptr_t)io->ioInterface.shutdown + offset);

	// Fix all addresses with in the DLDI
	if (io->fixSectionsFlags & FIX_ALL) {
		for (address = (u8**)io->dldiStart; address < (u8**)io->dldiEnd; address++) {
			if (oldStart <= *address && *address < oldEnd) {
				*address += offset;
			}
		}
	}
	
	// Fix the interworking glue section
	if (io->fixSectionsFlags & FIX_GLUE) {
		for (address = (u8**)io->interworkStart; address < (u8**)io->interworkEnd; address++) {
			if (oldStart <= *address && *address < oldEnd) {
				*address += offset;
			}
		}
	}
	
	// Fix the global offset table section
	if (io->fixSectionsFlags & FIX_GOT) {
		for (address = (u8**)io->gotStart; address < (u8**)io->gotEnd; address++) {
			if (oldStart <= *address && *address < oldEnd) {
				*address += offset;
			}
		}
	}
	
	// Initialise the BSS to 0
	if (io->fixSectionsFlags & FIX_BSS) {
		memset (io->bssStart, 0, (u8*)io->bssEnd - (u8*)io->bssStart);
	}
}
