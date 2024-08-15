/*
 disc_io.h
 Interface template for low level disc functions.

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

	2006-07-11 - Chishm
		* Original release
		
	2006-07-16 - Chishm
		* Renamed _CF_USE_DMA to _IO_USE_DMA
		* Renamed _CF_ALLOW_UNALIGNED to _IO_ALLOW_UNALIGNED
*/

#ifndef _DISC_IO_H
#define _DISC_IO_H

#include <nds/ndstypes.h>
#define BYTES_PER_SECTOR 512

//----------------------------------------------------------------------
// Customisable features

// Use DMA to read the card, remove this line to use normal reads/writes
// #define _IO_USE_DMA

// Allow buffers not alligned to 16 bits when reading files. 
// Note that this will slow down access speed, so only use if you have to.
// It is also incompatible with DMA
#define _IO_ALLOW_UNALIGNED

#if defined _IO_USE_DMA && defined _IO_ALLOW_UNALIGNED
 #error "You can't use both DMA and unaligned memory"
#endif

#define FEATURE_MEDIUM_CANREAD		0x00000001
#define FEATURE_MEDIUM_CANWRITE		0x00000002
#define FEATURE_SLOT_GBA			0x00000010
#define FEATURE_SLOT_NDS			0x00000020

typedef bool (* FN_MEDIUM_STARTUP)(void) ;
typedef bool (* FN_MEDIUM_ISINSERTED)(void) ;
typedef bool (* FN_MEDIUM_READSECTORS)(u32 sector, u32 numSectors, void* buffer) ;
typedef bool (* FN_MEDIUM_WRITESECTORS)(u32 sector, u32 numSectors, const void* buffer) ;
typedef bool (* FN_MEDIUM_CLEARSTATUS)(void) ;
typedef bool (* FN_MEDIUM_SHUTDOWN)(void) ;

struct IO_INTERFACE_STRUCT {
	unsigned long			ioType ;
	unsigned long			features ;
	FN_MEDIUM_STARTUP		fn_startup ;
	FN_MEDIUM_ISINSERTED	fn_isInserted ;
	FN_MEDIUM_READSECTORS	fn_readSectors ;
	FN_MEDIUM_WRITESECTORS	fn_writeSectors ;
	FN_MEDIUM_CLEARSTATUS	fn_clearStatus ;
	FN_MEDIUM_SHUTDOWN		fn_shutdown ;
} ;

typedef struct IO_INTERFACE_STRUCT IO_INTERFACE ;

#endif	// define _DISC_IO_H
