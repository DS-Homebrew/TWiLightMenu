////////////////////////////////////////////////////////////////////////////
//                           **** ADPCM-XQ ****                           //
//                  Xtreme Quality ADPCM Encoder/Decoder                  //
//                    Copyright (c) 2015 David Bryant.                    //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>

#include "adpcm-lib.h"

/* This module encodes and decodes 4-bit ADPCM (DVI/IMA varient). ADPCM data is divided
 * into independently decodable blocks that can be relatively small. The most common
 * configuration is to store 505 samples into a 256 byte block, although other sizes are
 * permitted as long as the number of samples is one greater than a multiple of 8. When
 * multiple channels are present, they are interleaved in the data with an 8-sample
 * interval. 
 */

/********************************* 4-bit ADPCM encoder ********************************/

#define CLIP(data, min, max) \
if ((data) > (max)) data = max; \
else if ((data) < (min)) data = min;

/* step table */
static const uint16_t step_table[89] = {
    7, 8, 9, 10, 11, 12, 13, 14,
    16, 17, 19, 21, 23, 25, 28, 31,
    34, 37, 41, 45, 50, 55, 60, 66,
    73, 80, 88, 97, 107, 118, 130, 143,
    157, 173, 190, 209, 230, 253, 279, 307,
    337, 371, 408, 449, 494, 544, 598, 658,
    724, 796, 876, 963, 1060, 1166, 1282, 1411,
    1552, 1707, 1878, 2066, 2272, 2499, 2749, 3024,
    3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484,
    7132, 7845, 8630, 9493, 10442, 11487, 12635, 13899,
    15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
    32767
};

/* step index tables */
static const int index_table[] = {
    /* adpcm data size is 4 */
    -1, -1, -1, -1, 2, 4, 6, 8
};

/********************************* 4-bit ADPCM decoder ********************************/

/* Decode the block of ADPCM data into PCM. This requires no context because ADPCM blocks
 * are indeppendently decodable. This assumes that a single entire block is always decoded;
 * it must be called multiple times for multiple blocks and cannot resume in the middle of a
 * block.
 *
 * Parameters:
 *  outbuf          destination for interleaved PCM samples
 *  inbuf           source ADPCM block
 *  inbufsize       size of source ADPCM block
 *  channels        number of channels in block (must be determined from other context)
 *
 * Returns number of converted composite samples (total samples divided by number of channels)
 */ 

int adpcm_decode_block (int16_t *outbuf, const uint8_t *inbuf, size_t inbufsize, int channels)
{
    int ch, samples = 1, chunks;
    int32_t pcmdata[2];
    int8_t index[2];

    if (inbufsize < (uint32_t) channels * 4)
        return 0;

    for (ch = 0; ch < channels; ch++) {
        *outbuf++ = pcmdata[ch] = (int16_t) (inbuf [0] | (inbuf [1] << 8));
        index[ch] = inbuf [2];

        if (index [ch] < 0 || index [ch] > 88 || inbuf [3])     // sanitize the input a little...
            return 0;

        inbufsize -= 4;
        inbuf += 4;
    }

    chunks = inbufsize / (channels * 4);
    samples += chunks * 8;

    while (chunks--) {
        int ch, i;

        for (ch = 0; ch < channels; ++ch) {

            for (i = 0; i < 4; ++i) {
                int step = step_table [index [ch]], delta = step >> 3;

                if (*inbuf & 1) delta += (step >> 2);
                if (*inbuf & 2) delta += (step >> 1);
                if (*inbuf & 4) delta += step;
                if (*inbuf & 8) delta = -delta;
                
                pcmdata[ch] += delta;
                index[ch] += index_table [*inbuf & 0x7];
                CLIP(index[ch], 0, 88);
                CLIP(pcmdata[ch], -32768, 32767);
                outbuf [i * 2 * channels] = pcmdata[ch];

                step = step_table [index [ch]], delta = step >> 3;

                if (*inbuf & 0x10) delta += (step >> 2);
                if (*inbuf & 0x20) delta += (step >> 1);
                if (*inbuf & 0x40) delta += step;
                if (*inbuf & 0x80) delta = -delta;
                
                pcmdata[ch] += delta;
                index[ch] += index_table [(*inbuf >> 4) & 0x7];
                CLIP(index[ch], 0, 88);
                CLIP(pcmdata[ch], -32768, 32767);
                outbuf [(i * 2 + 1) * channels] = pcmdata[ch];

                inbuf++;
            }

            outbuf++;
        }

        outbuf += channels * 7;
    }

    return samples;
}

