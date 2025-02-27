/*-----------------------------------------------------------------

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

#ifndef DLDI_PATCHER_H
#define DLDI_PATCHER_H

#include <nds/ndstypes.h>

typedef signed int addr_t;
typedef unsigned char data_t;
bool dldiPatchBinary (data_t *binData, u32 binSize);
void dldiDecompressBinary (void);
void dldiRelocateBinary (void);
void dldiClearBss (void);

#endif // DLDI_PATCHER_H
