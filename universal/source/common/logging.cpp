#include <nds.h>
#include <cstdio>
#include <stdarg.h>
#include "common/flashcard.h"
#include "common/twlmenusettings.h"

static const char* path = "";
static FILE* logFile;
static char logText[256];
static int position = 0;
static bool inited = false;

void logInit(void) {
	if (!ms().logging) {
		return;
	}

	path = sdFound() ? "sd:/_nds/TWiLightMenu/log.txt" : "fat:/_nds/TWiLightMenu/log.txt";
	logFile = fopen(path, "wb");
	if (!logFile) {
		return;
	}

	inited = true;
	sprintf(logText, "Logging Inited!");
	u16 newLine = 0x0A0D;

	fwrite(logText, 1, 15, logFile);
	fwrite(&newLine, sizeof(u16), 1, logFile);
	fwrite(&newLine, sizeof(u16), 1, logFile);
	fclose(logFile);

	position += 19;
}

void logPrint(const char* format, ...) {
	if (!inited) return;

	va_list args;
	va_start(args, format);
	vsnprintf(logText, sizeof(logText), format, args);
	va_end(args);

	int i = 0;
	for (i = 0; i < 255; i++) {
		if (logText[i]==0) {
			break;
		}
		if (logText[i]=='\x5C' && logText[i+1]=='n') {
			logText[i] = '\x0D';
			logText[i+1]='\x0A';
			i++;
			break;
		}
	}

	logFile = fopen(path, "r+");
	fseek(logFile, position, SEEK_SET);
	fwrite(logText, 1, i, logFile);
	fclose(logFile);

	position += i;
}
