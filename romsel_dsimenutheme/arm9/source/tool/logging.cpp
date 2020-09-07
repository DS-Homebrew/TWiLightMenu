#include <nds.h>
#include <cstdio>

static FILE* logFile;
static char logText[256];
static bool inited = false;

void logInit(void) {
	logFile = fopen("sd:/_nds/TWiLightMenu/log.txt", "wb");
	if (logFile) {
		inited = true;
		sprintf(logText, "Logging Inited!");
		fwrite(logText, 1, 15, logFile);
		u16 newLine = 0x0A0D;
		fwrite(&newLine, sizeof(u16), 1, logFile);
		fwrite(&newLine, sizeof(u16), 1, logFile);
		fclose(logFile);
	}
}

void logPrint(const char* text) {
	if (!inited) return;
	logFile = fopen("sd:/_nds/TWiLightMenu/log.txt", "wb");

	sprintf(logText, text);

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
	fwrite(logText, 1, i, logFile);
	fclose(logFile);
}