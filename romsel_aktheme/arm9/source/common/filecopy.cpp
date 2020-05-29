#include <nds.h>
#include <stdio.h>

#include "common/tonccpy.h"

u32 copyBuf[0x8000];

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

	off_t offset = 0;
	int numr;
	while (1)
	{
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
			return 0;
			break;
		}
	}

	return 1;
}

int fsizeincrease(const char *sourcePath, const char *tempPath, size_t newsize)
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

    FILE* destinationFile = fopen(tempPath, "wb");
	if (!destinationFile) {
		fclose(sourceFile);
		fclose(destinationFile);
		return 1;
	}

	off_t offset = 0;
	int numr;
	while (1)
	{
		// Copy file to destination path
		if (offset > fsize) {
			numr = 0x8000;
			toncset(copyBuf, 0, 0x8000);
		} else {
			numr = fread(copyBuf, 1, 0x8000, sourceFile);
		}
		fwrite(copyBuf, 1, numr, destinationFile);
		offset += 0x8000;

		if (offset > newsize) {
			fclose(sourceFile);
			fclose(destinationFile);
			remove(sourcePath);
			rename(tempPath, sourcePath);
			return 0;
			break;
		}
	}

	return 1;
}
