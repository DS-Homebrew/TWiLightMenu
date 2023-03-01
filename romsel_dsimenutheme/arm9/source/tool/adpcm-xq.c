////////////////////////////////////////////////////////////////////////////
//                           **** ADPCM-XQ ****                           //
//                  Xtreme Quality ADPCM Encoder/Decoder                  //
//                    Copyright (c) 2015 David Bryant.                    //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

#include <nds.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include "adpcm-lib.h"

extern bool fadeType;
extern bool showProgressIcon;
extern bool showProgressBar;
extern int progressBarLength;

/*static const char *sign_on = "\n"
" ADPCM-XQ   Xtreme Quality IMA-ADPCM WAV Encoder / Decoder   Version 0.3\n"
" Copyright (c) 2018 David Bryant. All Rights Reserved.\n\n";

static const char *usage =
" Usage:     ADPCM-XQ [-options] infile.wav outfile.wav\n\n"
" Operation: conversion is performed based on the type of the infile\n"
"          (either encode 16-bit PCM to 4-bit IMA-ADPCM or decode back)\n\n"
" Options:  -[0-8] = encode lookahead samples (default = 3)\n"
"           -bn    = override auto block size, 2^n bytes (n = 8-15)\n"
"           -d     = decode only (fail on WAV file already PCM)\n"
"           -e     = encode only (fail on WAV file already ADPCM)\n"
"           -f     = encode flat noise (no dynamic noise shaping)\n"
"           -h     = display this help message\n"
"           -q     = quiet mode (display errors only)\n"
"           -r     = raw output (no WAV header written)\n"
"           -v     = verbose (display lots of info)\n"
"           -y     = overwrite outfile if it exists\n\n"
" Web:       Visit www.github.com/dbry/adpcm-xq for latest version and info\n\n";*/

#define ADPCM_FLAG_NOISE_SHAPING    0x1
#define ADPCM_FLAG_RAW_OUTPUT       0x2

static int adpcm_converter (char *infilename, char *outfilename, int flags, int pcm8, int blocksize_pow2, int lookahead);
static int decode_only = 0, encode_only = 0;

int adpcm_main (const char* infilename, const char* outfilename, int pcm8)
{
    int lookahead = 3, flags = (ADPCM_FLAG_NOISE_SHAPING | ADPCM_FLAG_RAW_OUTPUT), blocksize_pow2 = 0;

    encode_only = 0;
    decode_only = 1;

    return adpcm_converter ((char*)infilename, (char*)outfilename, flags, pcm8, blocksize_pow2, lookahead);
}

typedef struct {
    char ckID [4];
    uint32_t ckSize;
    char formType [4];
} RiffChunkHeader;

typedef struct {
    char ckID [4];
    uint32_t ckSize;
} ChunkHeader;

#define ChunkHeaderFormat "4L"

typedef struct {
    uint16_t FormatTag, NumChannels;
    uint32_t SampleRate, BytesPerSecond;
    uint16_t BlockAlign, BitsPerSample;
    uint16_t cbSize;
    union {
        uint16_t ValidBitsPerSample;
        uint16_t SamplesPerBlock;
        uint16_t Reserved;
    } Samples;
    int32_t ChannelMask;
    uint16_t SubFormat;
    char GUID [14];
} WaveHeader;

#define WaveHeaderFormat "SSLLSSSSLS"

typedef struct {
    char ckID [4];
    uint32_t ckSize;
    uint32_t TotalSamples;
} FactHeader;

#define FactHeaderFormat "4LL"

#define WAVE_FORMAT_PCM         0x1
#define WAVE_FORMAT_IMA_ADPCM   0x11
#define WAVE_FORMAT_EXTENSIBLE  0xfffe

//static int write_pcm_wav_header (FILE *outfile, int num_channels, size_t num_samples, int sample_rate);
static int adpcm_decode_data (FILE *infile, FILE *outfile, int num_channels, size_t num_samples, int pcm8, int block_size);
static void little_endian_to_native (void *data, char *format);
//static void native_to_little_endian (void *data, char *format);

static int adpcm_converter (char *infilename, char *outfilename, int flags, int pcm8, int blocksize_pow2, int lookahead)
{
    int format = 0, res = 0, bits_per_sample, /*sample_rate,*/ num_channels;
    uint32_t fact_samples = 0;
    size_t num_samples = 0;
    FILE *infile, *outfile;
    RiffChunkHeader riff_chunk_header;
    ChunkHeader chunk_header;
    WaveHeader WaveHeader;

    if (!(infile = fopen (infilename, "rb"))) {
        //fprintf (stderr, "can't open file \"%s\" for reading!\n", infilename);
        return -1;
    }

    // read initial RIFF form header

    if (!fread (&riff_chunk_header, sizeof (RiffChunkHeader), 1, infile) ||
        strncmp (riff_chunk_header.ckID, "RIFF", 4) ||
        strncmp (riff_chunk_header.formType, "WAVE", 4)) {
            //fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
            return -1;
    }

    // Show progress bar
    showProgressIcon = true;
    showProgressBar = true;
    progressBarLength = 0;
    fadeType = true; // Fade in from white

    // loop through all elements of the RIFF wav header (until the data chuck)

    while (1) {

        if (!fread (&chunk_header, sizeof (ChunkHeader), 1, infile)) {
            //fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
            return -1;
        }

        little_endian_to_native (&chunk_header, ChunkHeaderFormat);

        // if it's the format chunk, we want to get some info out of there and
        // make sure it's a .wav file we can handle

        if (!strncmp (chunk_header.ckID, "fmt ", 4)) {
            int supported = 1;

            if (chunk_header.ckSize < 16 || chunk_header.ckSize > sizeof (WaveHeader) ||
                !fread (&WaveHeader, chunk_header.ckSize, 1, infile)) {
                    //fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                    return -1;
            }

            little_endian_to_native (&WaveHeader, WaveHeaderFormat);

            format = (WaveHeader.FormatTag == WAVE_FORMAT_EXTENSIBLE && chunk_header.ckSize == 40) ?
                WaveHeader.SubFormat : WaveHeader.FormatTag;

            bits_per_sample = (chunk_header.ckSize == 40 && WaveHeader.Samples.ValidBitsPerSample) ?
                WaveHeader.Samples.ValidBitsPerSample : WaveHeader.BitsPerSample;

            if (WaveHeader.NumChannels < 1 || WaveHeader.NumChannels > 2)
                supported = 0;
            else if (format == WAVE_FORMAT_IMA_ADPCM) {
                if (encode_only) {
                    //fprintf (stderr, "\"%s\" is ADPCM .WAV file, invalid in encode-only mode!\n", infilename);
                    return -1;
                }

                if (bits_per_sample != 4)
                    supported = 0;

                if (WaveHeader.Samples.SamplesPerBlock != (WaveHeader.BlockAlign - WaveHeader.NumChannels * 4) * (WaveHeader.NumChannels ^ 3) + 1) {
                    //fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                    return -1;
                }
            }
            else
                supported = 0;

            if (!supported) {
                //fprintf (stderr, "\"%s\" is an unsupported .WAV format!\n", infilename);
                return -1;
            }
        }
        else if (!strncmp (chunk_header.ckID, "fact", 4)) {

            if (chunk_header.ckSize < 4 || !fread (&fact_samples, sizeof (fact_samples), 1, infile)) {
                //fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                return -1;
            }

            if (chunk_header.ckSize > 4) {
                int bytes_to_skip = chunk_header.ckSize - 4;
                char dummy;

                while (bytes_to_skip--)
                    if (!fread (&dummy, 1, 1, infile)) {
                        //fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                        return -1;
                    }
            }
        }
        else if (!strncmp (chunk_header.ckID, "data", 4)) {

            // on the data chunk, get size and exit parsing loop

            if (!WaveHeader.NumChannels) {      // make sure we saw a "fmt" chunk...
                //fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                return -1;
            }

            if (!chunk_header.ckSize) {
                //fprintf (stderr, "this .WAV file has no audio samples, probably is corrupt!\n");
                return -1;
            }

            int complete_blocks = chunk_header.ckSize / WaveHeader.BlockAlign;
            int leftover_bytes = chunk_header.ckSize % WaveHeader.BlockAlign;
            int samples_last_block;

            num_samples = complete_blocks * WaveHeader.Samples.SamplesPerBlock;

            if (leftover_bytes) {
                if (leftover_bytes % (WaveHeader.NumChannels * 4)) {
                    //fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                    return -1;
                }
                samples_last_block = (leftover_bytes - (WaveHeader.NumChannels * 4)) * (WaveHeader.NumChannels ^ 3) + 1;
                num_samples += samples_last_block;
            }
            else
                samples_last_block = WaveHeader.Samples.SamplesPerBlock;

            if (fact_samples) {
                if (fact_samples < num_samples && fact_samples > num_samples - samples_last_block) {
                    num_samples = fact_samples;
                }
                else if (WaveHeader.NumChannels == 2 && (fact_samples >>= 1) < num_samples && fact_samples > num_samples - samples_last_block) {
                    num_samples = fact_samples;
                }
            }

            if (!num_samples) {
                //fprintf (stderr, "this .WAV file has no audio samples, probably is corrupt!\n");
                return -1;
            }

            num_channels = WaveHeader.NumChannels;
            // sample_rate = WaveHeader.SampleRate;
            break;
        }
        else {          // just ignore unknown chunks
            int bytes_to_eat = (chunk_header.ckSize + 1) & ~1L;
            char dummy;

            while (bytes_to_eat--)
                if (!fread (&dummy, 1, 1, infile)) {
                    //fprintf (stderr, "\"%s\" is not a valid .WAV file!\n", infilename);
                    return -1;
                }
        }
    }

    if (!(outfile = fopen (outfilename, "wb"))) {
        //fprintf (stderr, "can't open file \"%s\" for writing!\n", outfilename);
        return -1;
    }

    if (format == WAVE_FORMAT_IMA_ADPCM) {
        //if (!(flags & ADPCM_FLAG_RAW_OUTPUT) && !write_pcm_wav_header (outfile, num_channels, num_samples, sample_rate)) {
            //fprintf (stderr, "can't write header to file \"%s\" !\n", outfilename);
        //    return -1;
        //}

        res = adpcm_decode_data (infile, outfile, num_channels, num_samples, pcm8, WaveHeader.BlockAlign);
    }

    fclose (outfile);
    fclose (infile);

    // Fade out
    fadeType = false; // Fade to white
    for (int i = 0; i < 15; i++)
        swiWaitForVBlank();

    // Hide progress bar
    showProgressIcon = false;
    showProgressBar = false;
    progressBarLength = 0;

    return res;
}

/*static int write_pcm_wav_header (FILE *outfile, int num_channels, size_t num_samples, int sample_rate)
{
    RiffChunkHeader riffhdr;
    ChunkHeader datahdr, fmthdr;
    WaveHeader wavhdr;

    int wavhdrsize = 16;
    int bytes_per_sample = 2;
    size_t total_data_bytes = num_samples * bytes_per_sample * num_channels;

    memset (&wavhdr, 0, sizeof (wavhdr));

    wavhdr.FormatTag = WAVE_FORMAT_PCM;
    wavhdr.NumChannels = num_channels;
    wavhdr.SampleRate = sample_rate;
    wavhdr.BytesPerSecond = sample_rate * num_channels * bytes_per_sample;
    wavhdr.BlockAlign = bytes_per_sample * num_channels;
    wavhdr.BitsPerSample = 16;

    strncpy (riffhdr.ckID, "RIFF", sizeof (riffhdr.ckID));
    strncpy (riffhdr.formType, "WAVE", sizeof (riffhdr.formType));
    riffhdr.ckSize = sizeof (riffhdr) + wavhdrsize + sizeof (datahdr) + total_data_bytes;
    strncpy (fmthdr.ckID, "fmt ", sizeof (fmthdr.ckID));
    fmthdr.ckSize = wavhdrsize;

    strncpy (datahdr.ckID, "data", sizeof (datahdr.ckID));
    datahdr.ckSize = total_data_bytes;

    // write the RIFF chunks up to just before the data starts

    native_to_little_endian (&riffhdr, ChunkHeaderFormat);
    native_to_little_endian (&fmthdr, ChunkHeaderFormat);
    native_to_little_endian (&wavhdr, WaveHeaderFormat);
    native_to_little_endian (&datahdr, ChunkHeaderFormat);

    return fwrite (&riffhdr, sizeof (riffhdr), 1, outfile) &&
        fwrite (&fmthdr, sizeof (fmthdr), 1, outfile) &&
        fwrite (&wavhdr, wavhdrsize, 1, outfile) &&
        fwrite (&datahdr, sizeof (datahdr), 1, outfile);
}*/

static int adpcm_decode_data (FILE *infile, FILE *outfile, int num_channels, size_t num_samples, int pcm8, int block_size)
{
    int samples_per_block = (block_size - num_channels * 4) * (num_channels ^ 3) + 1;
    void *pcm_block = malloc (samples_per_block * num_channels * 2);
    void *pcm8_block = malloc (pcm8 ? (samples_per_block * num_channels) : 1);
    void *adpcm_block = malloc (block_size);
    int total_samples = num_samples;

    if (!pcm_block || !adpcm_block) {
        //fprintf (stderr, "could not allocate memory for buffers!\n");
        return -1;
    }

    while (num_samples) {
        int this_block_adpcm_samples = samples_per_block;
        int this_block_pcm_samples = samples_per_block;

        if (this_block_adpcm_samples > num_samples) {
            this_block_adpcm_samples = ((num_samples + 6) & ~7) + 1;
            block_size = (this_block_adpcm_samples - 1) / (num_channels ^ 3) + (num_channels * 4);
            this_block_pcm_samples = num_samples;
        }

        if (!fread (adpcm_block, block_size, 1, infile)) {
            //fprintf (stderr, "could not read all audio data from input file!\n");
            return -1;
        }

        if (adpcm_decode_block (pcm_block, adpcm_block, block_size, num_channels) != this_block_adpcm_samples) {
            //fprintf (stderr, "adpcm_decode_block() did not return expected value!\n");
            return -1;
        }

		if (pcm8) {
			// Convert PCM16 to PCM8
			for (int i = 0; i < samples_per_block * num_channels; i++) {
				int16_t sample = 0;
				memcpy(&sample, pcm_block+(i*2), 2);
				sample /= 0x100;
				memcpy(pcm8_block+i, &sample, 1);
			}
		}

        if (!fwrite (pcm8 ? pcm8_block : pcm_block, this_block_pcm_samples * (pcm8 ? num_channels : num_channels*2), 1, outfile)) {
            //fprintf (stderr, "could not write all audio data to output file!\n");
            return -1;
        }

        num_samples -= this_block_pcm_samples;

        progressBarLength = 192 * (total_samples - num_samples) / total_samples;
    }

    free (adpcm_block);
    free (pcm_block);
    free (pcm8_block);
    return 0;
}

static void little_endian_to_native (void *data, char *format)
{
    unsigned char *cp = (unsigned char *) data;
    int32_t temp;

    while (*format) {
        switch (*format) {
            case 'L':
                temp = cp [0] + ((int32_t) cp [1] << 8) + ((int32_t) cp [2] << 16) + ((int32_t) cp [3] << 24);
                * (int32_t *) cp = temp;
                cp += 4;
                break;

            case 'S':
                temp = cp [0] + (cp [1] << 8);
                * (short *) cp = (short) temp;
                cp += 2;
                break;

            default:
                if (isdigit ((unsigned char) *format))
                    cp += *format - '0';

                break;
        }

        format++;
    }
}

/*static void native_to_little_endian (void *data, char *format)
{
    unsigned char *cp = (unsigned char *) data;
    int32_t temp;

    while (*format) {
        switch (*format) {
            case 'L':
                temp = * (int32_t *) cp;
                *cp++ = (unsigned char) temp;
                *cp++ = (unsigned char) (temp >> 8);
                *cp++ = (unsigned char) (temp >> 16);
                *cp++ = (unsigned char) (temp >> 24);
                break;

            case 'S':
                temp = * (short *) cp;
                *cp++ = (unsigned char) temp;
                *cp++ = (unsigned char) (temp >> 8);
                break;

            default:
                if (isdigit ((unsigned char) *format))
                    cp += *format - '0';

                break;
        }

        format++;
    }
}*/

