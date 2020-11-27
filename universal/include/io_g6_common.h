/*
	iointerface.c for G6 flash card

Written by Viruseb (viruseb@hotmail.com)
	Many thanks to Puyo for his help in the reading code and ecc generation
	and Theli for remote debbuging.

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
 
If you wish to understand how the code works, I encourage you reading 
the datasheet of K9K4G08U0M nand flash device from Samsung before.

Just some figures to keep in mind :
	1 page  = 4 sectors + 64byte
	1 block = 64 pages = 256 sectors
	1 4G device = 4096 blocks

The spare 64byte in page are use :
	- to store the ECC. There is 3bytes ecc per 256bytes (bytes 8...15+numsector*16).
	- to store lookuptable values (bytes 4..7).
	
04/12/06 : Version 0.10
	Just freshly written. Not tested on real G6L thought. 
	Extreme caution have to be taken when testing WriteBlockFunction 
	since it may brick your G6L or more likely corrupt your data 
	and formating can be necessary.
	
05/12/06 : Version 0.11
	Thank to Theli, a lot of stupid mistakes removed, a lot of debugging done.
	Reading code checked against Puyo's code. 
	Device and FAT table is recognised by Fat_InitFiles()
	Known issues : DMA read (G6_ReadDMA) is malfunctionning
				Strange things append when trying to read more than 1 sectors at a time
				Have to check LookUpTable values against Puyo's LookUpTable they seems differents after 512values 
	
19/12/06 : Version 0.12
	Reading code ok
	
20/12/06 : Version 0.13
	Some reading bugs corrected
	
07/01/07 : Version 0.14
	Writing code finaly working. Need some optimizations.
	
10/01/07 : Version 0.15
	Code cleaning. Need to add DMA R/W and use of cache programming in later version. 
	
03/02/07 : Version 0.16
	Unaligned R/W supported.
	Write code rewritten, using cache programming now.

04/03/07 : Version 0.17
	VerifyBlockFunc corrected (no more in use now)
	
23/03/07 : Version 0.18
	a bug corrected in make_ecc_256
	
25/03/07 : Version 0.19
	Improved writing speed
*/

#ifndef IO_G6_COMMON_H
#define IO_G6_COMMON_H

#include <nds/ndstypes.h>

#ifdef __cplusplus
extern "C" {
#endif

// Values for changing mode
#define G6_MODE_RAM 6
#define G6_MODE_MEDIA 3 

extern void _G6_SelectOperation(u16 op);

#ifdef __cplusplus
}
#endif

#endif // IO_G6_COMMON_H

