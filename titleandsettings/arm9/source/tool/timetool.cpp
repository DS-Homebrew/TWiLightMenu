/*---------------------------------------------------------------------------------


Copyright (C) 2007 Acekard, www.acekard.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.


---------------------------------------------------------------------------------*/

#include <nds.h>

void waitMs(unsigned int requestTime)
{
    unsigned int lastLine = REG_VCOUNT;
    unsigned int newLine;
    unsigned int elapsedTime = 0;  // in ms
    unsigned int elapsedLines = 0; // in lines

    while (elapsedTime < requestTime)
    {
        int diffLine;
        newLine = REG_VCOUNT;

        diffLine = newLine - lastLine;
        if (diffLine < 0)
            diffLine = 263 + diffLine;

        elapsedLines += diffLine;

        //does this correctly optimize?
        //elapsedTime = elapsedLines/16; // 16 lines = 1ms
        elapsedTime = elapsedLines >> 4; // 16 lines = 1ms

        lastLine = newLine;
    }
}
