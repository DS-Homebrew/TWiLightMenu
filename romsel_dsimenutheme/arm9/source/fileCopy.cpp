#include <nds.h>
#include <stdio.h>

#include "common/tonccpy.h"

extern bool showProgressBar;
extern int progressBarLength;

off_t getFileSize(const char *fileName)
{
	FILE* fp = fopen(fileName, "rb");
	off_t fsize = 0;
	if (fp) {
		fseek(fp, 0, SEEK_END);
		fsize = ftell(fp);			// Get source file's size
		fseek(fp, 0, SEEK_SET);
	}
	fclose(fp);

	return fsize;
}

char copyBuf[0x8000];

int fcopy(const char *sourcePath, const char *destinationPath)
{
	FILE* sourceFile = fopen(sourcePath, "rb");
	off_t fsize = 0;
	if (sourceFile) {
		fseek(sourceFile, 0, SEEK_END);
		fsize = ftell(sourceFile);			// Get source file's size
		fseek(sourceFile, 0, SEEK_SET);
	} else {
		fclose(sourceFile);
		return 1;
	}

	FILE* destinationFile = fopen(destinationPath, "wb");
	if (!destinationFile) {
		fclose(sourceFile);
		fclose(destinationFile);
		return 1;
	}

	showProgressBar = true;
	progressBarLength = 0;
	off_t offset = 0;
	int numr;
	while (1) {
		/* scanKeys();
		if (keysHeld() & KEY_A) {
			// Cancel copying
			fclose(sourceFile);
			fclose(destinationFile);
			return -1;
			break;
		} */

		// Copy file to destination path
		numr = fread(copyBuf, 1, 0x8000, sourceFile);
		fwrite(copyBuf, 1, numr, destinationFile);
		offset += 0x8000;

		if (offset > fsize) {
			fclose(sourceFile);
			fclose(destinationFile);
			showProgressBar = false;
			return 0;
		}
		progressBarLength = (offset+0x8000)/(fsize/192);
	}

	showProgressBar = false;
	return 1;
}
