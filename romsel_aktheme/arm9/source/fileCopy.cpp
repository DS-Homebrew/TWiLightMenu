#include <nds.h>
#include <stdio.h>

#include "graphics/graphics.h"
#include "common/tonccpy.h"

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
	displayDiskIcon(strncmp("fat:", sourcePath, 4) == 0 || strncmp("fat:", destinationPath, 4) == 0);

	FILE* sourceFile = fopen(sourcePath, "rb");
	off_t fsize = 0;
	if (sourceFile) {
		fseek(sourceFile, 0, SEEK_END);
		fsize = ftell(sourceFile);			// Get source file's size
		fseek(sourceFile, 0, SEEK_SET);
	} else {
		fclose(sourceFile);
		displayDiskIcon(false);
		return 1;
	}

	FILE* destinationFile = fopen(destinationPath, "wb");
	if (!destinationFile) {
		fclose(sourceFile);
		fclose(destinationFile);
		displayDiskIcon(false);
		return 1;
	}

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
			displayDiskIcon(false);
			return 0;
		}
	}

	displayDiskIcon(false);
	return 1;
}
