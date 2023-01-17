////////////////////////////////////////////////////////////////////////////
//                           **** ADPCM-XQ ****                           //
//                  Xtreme Quality ADPCM Encoder/Decoder                  //
//                    Copyright (c) 2015 David Bryant.                    //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

#ifndef ADPCMXQ_H_
#define ADPCMXQ_H_

#ifdef __cplusplus
extern "C" {
#endif

int adpcm_main (const char* infilename, int pcm8, bool loopingPoint);
int adpcm_decode_data (FILE *infile, void *buffer, int instance_to_fill, int num_channels, int pcm8, bool loopingPoint);

#ifdef __cplusplus
}
#endif

#endif /* ADPCMXQ_H_ */
