/*-----------------------------------------------------------------
 fat.h
 
 NDS MP
 GBAMP NDS Firmware Hack Version 2.12
 An NDS aware firmware patch for the GBA Movie Player.
 By Michael Chisholm (Chishm)
 
 Filesystem code based on GBAMP_CF.c by Chishm (me).
 
License:
 Copyright (C) 2005  Michael "Chishm" Chisholm

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

 If you use this code, please give due credit and email me about your
 project at chishm@hotmail.com
------------------------------------------------------------------*/

#ifndef FAT_H
#define FAT_H

#include <nds/ndstypes.h>

#define CLUSTER_FREE	0x00000000
#define	CLUSTER_EOF		0x0FFFFFFF
#define CLUSTER_FIRST	0x00000002

bool FAT_InitFiles (bool initCard);
u32 getBootFileCluster (const char* bootName);
u32 fileRead (char* buffer, u32 cluster, u32 startOffset, u32 length);
u32 FAT_ClustToSect (u32 cluster);

#endif // FAT_H
