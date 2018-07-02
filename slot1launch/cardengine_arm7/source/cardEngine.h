/*
    NitroHax -- Cheat tool for the Nintendo DS
    Copyright (C) 2008  Michael "Chishm" Chisholm

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CARD_ENGINE_ARM7_H
#define CARD_ENGINE_ARM7_H

#ifdef __cplusplus
extern "C" {
#endif

#define is_aligned(POINTER, BYTE_COUNT) \
    (((uintptr_t)(const void *)(POINTER)) % (BYTE_COUNT) == 0)

void myIrqHandlerFIFO(void);

u32 myIrqEnable(u32 irq);

/*void runCardEngineCheck (void);*/

bool eepromProtect (void);
bool eepromRead (u32 src, void *dst, u32 len);
bool eepromPageWrite (u32 dst, const void *src, u32 len);
bool eepromPageProg (u32 dst, const void *src, u32 len);
bool eepromPageVerify (u32 dst, const void *src, u32 len);
bool eepromPageErase (u32 dst);

u32 cardId (void);
bool cardRead (u32 dma,  u32 src, void *dst, u32 len);

#ifdef __cplusplus
}
#endif

#endif // CARD_ENGINE_ARM7_H
