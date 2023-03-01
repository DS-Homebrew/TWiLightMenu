#include "sound.h"
#include "tool/adpcm-xq.h"

#include "graphics/themefilenames.h"
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "graphics/fontHandler.h"
#include "language.h"
#include "fileCopy.h"
#include "streamingaudio.h"
#include "string.h"
#include "common/tonccpy.h"
#include <algorithm>
#include <sys/stat.h>

#define SFX_STARTUP		0
#define SFX_WRONG		1
#define SFX_LAUNCH		2
#define SFX_STOP		3
#define SFX_SWITCH		4
#define SFX_SELECT		5
#define SFX_BACK		6

#define MSL_NSONGS		0
#define MSL_NSAMPS		7
#define MSL_BANKSIZE	7


extern bool controlTopBright;

extern volatile s16 fade_counter;
extern volatile bool fade_out;

extern volatile s16* play_stream_buf;
extern volatile s16* fill_stream_buf;

// Number of samples filled into the fill buffer so far.
extern volatile s32 filled_samples;

extern volatile bool fill_requested;
extern volatile s32 samples_left_until_next_fill;
extern volatile s32 streaming_buf_ptr;

#define SAMPLES_USED (STREAMING_BUF_LENGTH - samples_left)
#define REFILL_THRESHOLD STREAMING_BUF_LENGTH >> 2

#ifdef SOUND_DEBUG
extern char debug_buf[256];
#endif

extern volatile u32 sample_delay_count;

volatile char* SFX_DATA = (char*)NULL;
mm_word SOUNDBANK[MSL_BANKSIZE] = {0};

SoundControl::SoundControl()
	: stream_is_playing(false), stream_source(NULL), startup_sample_length(0), seekPos(0)
 {

	sndSys.mod_count = MSL_NSONGS;
	sndSys.samp_count = MSL_NSAMPS;
	sndSys.mem_bank = SOUNDBANK;
	sndSys.fifo_channel = FIFO_MAXMOD;

	FILE* soundbank_file;

	if (ms().theme == TWLSettings::EThemeSaturn) {
		soundbank_file = fopen(std::string(TFN_SATURN_SOUND_EFFECTBANK).c_str(), "rb");
	} else {
		switch(ms().dsiMusic) {
			case 3:
				soundbank_file = fopen(std::string(TFN_SOUND_EFFECTBANK).c_str(), "rb");
				if (soundbank_file) break; // fallthrough if soundbank_file fails.
			case 1:
			case 2:
			default:
				soundbank_file = fopen(std::string(TFN_DEFAULT_SOUND_EFFECTBANK).c_str(), "rb");
				break;
		}
	}

	fseek(soundbank_file, 0, SEEK_END);
	size_t sfxDataSize = ftell(soundbank_file);
	fseek(soundbank_file, 0, SEEK_SET);

	SFX_DATA = new char[sfxDataSize > 0x7D000 ? 0x7D000 : sfxDataSize];
	fread((void*)SFX_DATA, 1, sfxDataSize, soundbank_file);

	fclose(soundbank_file);

	// Since SFX_STARTUP is the first sample, it begins at 0x10 after the
	// *maxmod* header. Subtract the size of the sample header,
	// and divide by two to get length in samples.
	// https://github.com/devkitPro/mmutil/blob/master/source/msl.c#L80
	
	startup_sample_length = (((*(u32*)(SFX_DATA + 0x10)) - 20) >> 1);

	// sprintf(debug_buf, "Read sample length %li for startup", startup_sample_length);
    // nocashMessage(debug_buf);

	mmInit(&sndSys);
	mmSoundBankInMemory((mm_addr)SFX_DATA);

	mmLoadEffect(SFX_LAUNCH);
	mmLoadEffect(SFX_SELECT);
	mmLoadEffect(SFX_STOP);
	mmLoadEffect(SFX_WRONG);
	mmLoadEffect(SFX_BACK);
	mmLoadEffect(SFX_SWITCH);
	mmLoadEffect(SFX_STARTUP);
	// mmLoadEffect(SFX_MENU);

	snd_launch = {
	    {SFX_LAUNCH},	    // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_select = {
	    {SFX_SELECT},	    // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_stop = {
	    {SFX_STOP},		     // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_wrong = {
	    {SFX_WRONG},	     // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_back = {
	    {SFX_BACK},		     // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	snd_switch = {
	    {SFX_SWITCH},	    // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};
	mus_startup = {
	    {SFX_STARTUP},	   // id
	    (int)(1.0f * (1 << 10)), // rate
	    0,			     // handle
	    255,		     // volume
	    128,		     // panning
	};


	if (ms().dsiMusic == 0 || ms().theme == TWLSettings::EThemeSaturn) {
		return;
	}

	bool loopableMusic = false;
	loopingPoint = false;

	mkdir(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/cache" : "fat:/_nds/TWiLightMenu/cache", 0777);
	mkdir(sys().isRunFromSD() ? "sd:/_nds/TWiLightMenu/cache/music" : "fat:/_nds/TWiLightMenu/cache/music", 0777);
	std::string devicePath = sys().isRunFromSD() ? "sd:" : "fat:";

	stream.sampling_rate = 16000;	 		// 16000Hz
	stream.format = MM_STREAM_16BIT_MONO;  // select format

	// Leave top scren white
	controlTopBright = false;

	printLarge(false, 0, 88 - (calcLargeFontHeight(STR_PREPARING_MUSIC) - largeFontHeight()) / 2, STR_PREPARING_MUSIC, Alignment::center);
	updateText(false);

	if (ms().theme == TWLSettings::EThemeSaturn) {
		stream_source = fopen(std::string(TFN_DEFAULT_SOUND_BG).c_str(), "rb");
	} else {
		switch(ms().dsiMusic) {
			case 5: { // HBL
				std::string startPath = (devicePath+TFN_HBL_START_SOUND_BG_CACHE);
				if (access(startPath.c_str(), F_OK) != 0 || getFileSize(startPath.c_str()) == 0) {
					if (adpcm_main(std::string(TFN_HBL_START_SOUND_BG).c_str(), startPath.c_str(), true) == -1) {
						remove(startPath.c_str());
					}
				}
				std::string loopPath = (devicePath+TFN_HBL_LOOP_SOUND_BG_CACHE);
				if (access(loopPath.c_str(), F_OK) != 0 || getFileSize(loopPath.c_str()) == 0) {
					if (adpcm_main(std::string(TFN_HBL_LOOP_SOUND_BG).c_str(), loopPath.c_str(), true) == -1) {
						remove(startPath.c_str());
						remove(loopPath.c_str());
					}
				}
				stream.sampling_rate = 44100;	 		// 44100Hz
				stream.format = MM_STREAM_8BIT_STEREO;
				stream_start_source = fopen(startPath.c_str(), "rb");
				stream_source = fopen(loopPath.c_str(), "rb");
				loopableMusic = true;
				break; }
			case 4:
			case 2: { // DSi Shop
				std::string startPath = (devicePath+TFN_SHOP_START_SOUND_BG_CACHE);
				if (access(startPath.c_str(), F_OK) != 0 || getFileSize(startPath.c_str()) == 0) {
					if (adpcm_main(std::string(TFN_SHOP_START_SOUND_BG).c_str(), startPath.c_str(), true) == -1) {
						remove(startPath.c_str());
					}
				}
				std::string loopPath = (devicePath+TFN_SHOP_LOOP_SOUND_BG_CACHE);
				if (access(loopPath.c_str(), F_OK) != 0 || getFileSize(loopPath.c_str()) == 0) {
					if (adpcm_main(std::string(TFN_SHOP_LOOP_SOUND_BG).c_str(), loopPath.c_str(), true) == -1) {
						remove(startPath.c_str());
						remove(loopPath.c_str());
					}
				}
				stream.sampling_rate = 44100;	 		// 44100Hz
				stream.format = MM_STREAM_8BIT_STEREO;
				stream_start_source = fopen(startPath.c_str(), "rb");
				stream_source = fopen(loopPath.c_str(), "rb");
				loopableMusic = true;
				break; }
			case 3: { // Theme
				bool customSkin = false;
				switch (ms().theme) {
					case 0:
					default:
						customSkin = (tfn().uiDirectory() != TFN_FALLBACK_DSI_UI_DIRECTORY);
						break;
					case 1:
						customSkin = (tfn().uiDirectory() != TFN_FALLBACK_3DS_UI_DIRECTORY);
						break;
					//case 4:
					//	customSkin = (tfn().uiDirectory() != TFN_FALLBACK_SATURN_UI_DIRECTORY);
					//	break;
					case 5:
						customSkin = (tfn().uiDirectory() != TFN_FALLBACK_HBLAUNCHER_UI_DIRECTORY);
						break;
				}
				if (customSkin) {
					std::string musicPath = TFN_SOUND_BG;
					std::string cachePath = TFN_SOUND_BG_CACHE;
					bool closed = false;
					if (access(musicPath.c_str(), F_OK) == 0) {
						// Read properties from WAV header
						u8 wavFormat = 0;
						u8 numChannels = 1;
						stream_source = fopen(musicPath.c_str(), "rb");
						fseek(stream_source, 0x14, SEEK_SET);
						fread(&wavFormat, sizeof(u8), 1, stream_source);
						fseek(stream_source, 0x16, SEEK_SET);
						fread(&numChannels, sizeof(u8), 1, stream_source);
						stream.format = numChannels == 2 ? MM_STREAM_8BIT_STEREO : MM_STREAM_16BIT_MONO;
						fseek(stream_source, 0x18, SEEK_SET);
						fread(&stream.sampling_rate, sizeof(u16), 1, stream_source);

						if (wavFormat == 0x11) {
							// If ADPCM and hasn't been successfully converted yet, do so now
							if(access(cachePath.c_str(), F_OK) != 0 || getFileSize(cachePath.c_str()) == 0) {
								if (adpcm_main(musicPath.c_str(), cachePath.c_str(), numChannels == 2) == -1) {
									remove(cachePath.c_str());
								}
							}
							fclose(stream_source);
							closed = true;
						} else {
							seekPos = 0x2C;
						}
					} else {
						closed = true;
					}
					if (closed) stream_source = fopen(cachePath.c_str(), "rb");
					if (stream_source) break; } // fallthrough if stream_source fails.
				}
			case 1:
			default: {
				std::string musicPath = (devicePath+TFN_DEFAULT_SOUND_BG_CACHE);
				if (access(musicPath.c_str(), F_OK) != 0 || getFileSize(musicPath.c_str()) == 0) {
					if (adpcm_main(std::string(TFN_DEFAULT_SOUND_BG).c_str(), musicPath.c_str(), true) == -1) {
						remove(musicPath.c_str());
					}
				}
				stream.sampling_rate = 32000;	 		// 32000Hz
				stream.format = MM_STREAM_8BIT_STEREO;
				stream_source = fopen(musicPath.c_str(), "rb");
				break; }
		}
	}

	clearText();
	controlTopBright = true;

	fseek(stream_source, seekPos, SEEK_SET);

	stream.buffer_length = 0x1000;	  			// should be adequate
	stream.callback = on_stream_request;    
	stream.timer = MM_TIMER0;	    	   // use timer0
	stream.manual = false;	      		   // auto filling

	if (loopableMusic) {
		fseek(stream_start_source, 0, SEEK_END);
		size_t fileSize = ftell(stream_start_source);
		fseek(stream_start_source, 0, SEEK_SET);

		// Prep the first section of the stream
		fread((void*)play_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_start_source);
		if (fileSize < STREAMING_BUF_LENGTH*sizeof(s16)) {
			size_t fillerSize = 0;
			while (fileSize+fillerSize < STREAMING_BUF_LENGTH*sizeof(s16)) {
				fillerSize++;
			}
			fread((void*)(play_stream_buf+fileSize), 1, fillerSize, stream_source);

			// Fill the next section premptively
			fread((void*)fill_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source);

			loopingPoint = true;
		} else {
			// Fill the next section premptively
			fread((void*)fill_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_start_source);
			fileSize -= STREAMING_BUF_LENGTH*sizeof(s16);
			if (fileSize < STREAMING_BUF_LENGTH*sizeof(s16)) {
				size_t fillerSize = 0;
				while (fileSize+fillerSize < STREAMING_BUF_LENGTH*sizeof(s16)) {
					fillerSize++;
				}
				fread((void*)(fill_stream_buf+fileSize), 1, fillerSize, stream_source);

				loopingPoint = true;
			}
		}
	} else {
		// Prep the first section of the stream
		fread((void*)play_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source);

		// Fill the next section premptively
		fread((void*)fill_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source);

		loopingPoint = true;
	}
}

mm_sfxhand SoundControl::playLaunch() { return mmEffectEx(&snd_launch); }
mm_sfxhand SoundControl::playSelect() { return mmEffectEx(&snd_select); }
mm_sfxhand SoundControl::playBack() { return mmEffectEx(&snd_back); }
mm_sfxhand SoundControl::playSwitch() { return mmEffectEx(&snd_switch); }
mm_sfxhand SoundControl::playStartup() { return mmEffectEx(&mus_startup); }
mm_sfxhand SoundControl::playStop() { return mmEffectEx(&snd_stop); }
mm_sfxhand SoundControl::playWrong() { return mmEffectEx(&snd_wrong); }

void SoundControl::beginStream() {
	if (!stream_source) return;

	// open the stream
	stream_is_playing = true;
	mmStreamOpen(&stream);
	SetYtrigger(0);
}

void SoundControl::stopStream() {
	if (!stream_source) return;

	stream_is_playing = false;
	mmStreamClose();
}

void SoundControl::fadeOutStream() {
	//fade_out = true; // Bugged
	fifoSendValue32(FIFO_USER_01, 1); // Fade out on ARM7 side
}

void SoundControl::cancelFadeOutStream() {
	//fade_out = false;
	//fade_counter = FADE_STEPS;
}

void SoundControl::setStreamDelay(u32 delay) {
	sample_delay_count = delay;
}


// Samples remaining in the fill buffer.
#define SAMPLES_LEFT_TO_FILL (abs(STREAMING_BUF_LENGTH - filled_samples))

// Samples that were already streamed and need to be refilled into the buffer.
#define SAMPLES_TO_FILL (abs(streaming_buf_ptr - filled_samples))

// Updates the background music fill buffer
// Fill the amount of samples that were used up between the
// last fill request and this.

// Precondition Invariants:
// filled_samples <= STREAMING_BUF_LENGTH
// filled_samples <= streaming_buf_ptr

// Postcondition Invariants:
// filled_samples <= STREAMING_BUF_LENGTH
// filled_samples <= streaming_buf_ptr
// fill_requested == false
volatile void SoundControl::updateStream() {
	
	if (!stream_is_playing) return;
	if (fill_requested && filled_samples < STREAMING_BUF_LENGTH) {
			
		// Reset the fill request
		fill_requested = false;
		int instance_filled = 0;

		// Either fill the max amount, or fill up the buffer as much as possible.
		int instance_to_fill = std::min(SAMPLES_LEFT_TO_FILL, SAMPLES_TO_FILL);

		// If we don't read enough samples, loop from the beginning of the file.
		instance_filled = fread((s16*)fill_stream_buf + filled_samples, sizeof(s16), instance_to_fill, loopingPoint ? stream_source : stream_start_source);		
		if (instance_filled < instance_to_fill) {
			fseek(stream_source, seekPos, SEEK_SET);
			instance_filled += fread((s16*)fill_stream_buf + filled_samples + instance_filled,
				 sizeof(s16), (instance_to_fill - instance_filled), stream_source);
			loopingPoint = true;
		}

		#ifdef SOUND_DEBUG
		sprintf(debug_buf, "FC: SAMPLES_LEFT_TO_FILL: %li, SAMPLES_TO_FILL: %li, instance_filled: %i, filled_samples %li, to_fill: %i", SAMPLES_LEFT_TO_FILL, SAMPLES_TO_FILL, instance_filled, filled_samples, instance_to_fill);
    	nocashMessage(debug_buf);
		#endif

		// maintain invariant 0 < filled_samples <= STREAMING_BUF_LENGTH
		filled_samples = std::min<s32>(filled_samples + instance_filled, STREAMING_BUF_LENGTH);

	
	} else if (fill_requested && filled_samples >= STREAMING_BUF_LENGTH) {
		// filled_samples == STREAMING_BUF_LENGTH is the only possible case
		// but we'll keep it at gte to be safe.
		filled_samples = 0;
		// fill_count = 0;
	}

}
