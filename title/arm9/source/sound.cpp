#include "sound.h"

#include "common/twlmenusettings.h"
#include "common/flashcard.h"
#include "common/systemdetails.h"
#include "streamingaudio.h"
#include "string.h"
#include "common/tonccpy.h"
#include <algorithm>
#include <sys/stat.h>

#include "soundbank.h"


// Reference: http://yannesposito.com/Scratch/en/blog/2010-10-14-Fun-with-wav/
typedef struct _WavHeader {
	char magic[4];		// "RIFF"
	u32 totallength;	// Total file length, minus 8.
	char wavefmt[8];	// Should be "WAVEfmt "
	u32 format;		// 16 for PCM format
	u16 pcm;		// 1 for PCM format
	u16 channels;		// Channels
	u32 frequency;		// Sampling frequency
	u32 bytes_per_second;
	u16 bytes_by_capture;
	u16 bits_per_sample;
	char data[4];		// "data"
	u32 bytes_in_data;
} WavHeader;

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

char soundBank[0x4C000] = {0};
bool soundBankInited = false;

SoundControl::SoundControl()
	: stream_is_playing(false), stream_source(NULL)
 {
	if (soundBankInited) {
		return;
	}

	// Get date
	extern bool useTwlCfg;
	int birthMonth = (useTwlCfg ? *(u8*)0x02000446 : PersonalData->birthMonth);
	int birthDay = (useTwlCfg ? *(u8*)0x02000447 : PersonalData->birthDay);
	char soundBankPath[32], currentDate[16], birthDate[16];
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	strftime(currentDate, sizeof(currentDate), "%m/%d", Time);
	sprintf(birthDate, "%02d/%02d", birthMonth, birthDay);

	sprintf(soundBankPath, "nitro:/soundbank%s.bin", (strcmp(currentDate, birthDate) == 0) ? "_bday" : "");

	// Load sound bank into memory
	FILE* soundBankF = fopen(soundBankPath, "rb");
	fread(soundBank, 1, sizeof(soundBank), soundBankF);
	fclose(soundBankF);

	mmInitDefaultMem((mm_addr)soundBank);

	mmLoadEffect( sys().isRegularDS() ? SFX_DSBOOT : SFX_DSIBOOT );
	mmLoadEffect( SFX_SELECT );

	if (sys().isRegularDS()) {
		snd_dsiboot = {
			{ SFX_DSBOOT } ,			// id
			(int)(1.0f * (1<<10)),	// rate
			0,		// handle
			255,	// volume
			128,	// panning
		};
	} else {
		snd_dsiboot = {
			{ SFX_DSIBOOT } ,			// id
			(int)(1.0f * (1<<10)),	// rate
			0,		// handle
			255,	// volume
			128,	// panning
		};
	}

	snd_select = {
		{ SFX_SELECT } ,			// id
		(int)(1.0f * (1<<10)),	// rate
		0,		// handle
		255,	// volume
		128,	// panning
	};


	stream_source = fopen("nitro:/title.wav", "rb");


	WavHeader wavHeader;
	fread(&wavHeader, 1, sizeof(wavHeader), stream_source);

	stream.sampling_rate = wavHeader.frequency;
	stream.format = MM_STREAM_16BIT_MONO;  // select format
	stream.buffer_length = 0x1000;	  			// should be adequate
	stream.callback = on_stream_request;    
	stream.timer = MM_TIMER0;	    	   // use timer0
	stream.manual = false;	      		   // auto filling

	// Prep the first section of the stream
	fread((void*)play_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source);

	// Fill the next section premptively
	fread((void*)fill_stream_buf, sizeof(s16), STREAMING_BUF_LENGTH, stream_source);

	soundBankInited = true;
}

mm_sfxhand SoundControl::playDSiBoot() { return mmEffectEx(&snd_dsiboot); }
mm_sfxhand SoundControl::playSelect() { return mmEffectEx(&snd_select); }

void SoundControl::beginStream() {
	// open the stream
	stream_is_playing = true;
	mmStreamOpen(&stream);
	SetYtrigger(0);
}

void SoundControl::stopStream() {
	stream_is_playing = false;
	mmStreamClose();
}

void SoundControl::fadeOutStream() {
	//fade_out = true; // Bugged
	fifoSendValue32(FIFO_USER_02, 1); // Fade out on ARM7 side
}

void SoundControl::cancelFadeOutStream() {
	//fade_out = false;
	//fade_counter = FADE_STEPS;
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
		instance_filled = fread((s16*)fill_stream_buf + filled_samples, sizeof(s16), instance_to_fill, stream_source);		
		if (instance_filled < instance_to_fill) {
			toncset((s16*)fill_stream_buf + filled_samples + instance_filled, 0, (instance_to_fill - instance_filled)*sizeof(s16));
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
