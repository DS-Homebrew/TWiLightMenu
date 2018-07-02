/*---------------------------------------------------------------------------------

	Copyright (C) 2008 - 2010
		Mukunda Johnson (eKid)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
        .text
        .thumb
        .align

        .thumb_func
//---------------------------------------------------------------------------------
        .global nocashWrite
//---------------------------------------------------------------------------------
// no$gba debug output function
//---------------------------------------------------------------------------------
nocashWrite:  // params = { string, length }
//---------------------------------------------------------------------------------
	b	1f

        .thumb_func
//---------------------------------------------------------------------------------
        .global nocashMessage
//---------------------------------------------------------------------------------
// no$gba debug output function
//---------------------------------------------------------------------------------
nocashMessage:  // params = { string }
//---------------------------------------------------------------------------------
 
// max string length == 120 bytes
//---------------------------------------------------------------------------------
// copy string into buffer
//---------------------------------------------------------------------------------
	mov	r1,#120
1:	push	{r4}
        ldr     r4,=buffer  // get buffer address
        mov     r2, #0      // r2 = read/write position
3:      ldrb    r3, [r0,r2] // load character
        strb    r3, [r4,r2] // store character
        cmp     r3, #0      // character == NULL?
        beq     3f          // yes, send message
        add     r2, #1      // increment read/write position
        cmp     r2, r1      // max length == 120
        bne     3b          // loop if < 120 characters
 
	mov	r0,#0
	strb	r0,[r4,r2]
//---------------------------------------------------------------------------------
// send message to no$
//---------------------------------------------------------------------------------
3:      mov     r12,r12     // first ID
        b       2f          // skip the text section
        .hword  0x6464      // second ID
        .hword  0           // flags
buffer:
        .space  120         // data
2:	pop	{r4}
	bx      lr          // exit
 
//---------------------------------------------------------------------------------
        .pool
