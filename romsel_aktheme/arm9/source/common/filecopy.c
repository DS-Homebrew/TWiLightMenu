#include <nds.h>
#include <stdio.h>
#include <sys/stat.h>

#include "common/tonccpy.h"
#include "_ansi.h"

#define COPY_BUF_SIZE 0x8000
u8 copyBuf[COPY_BUF_SIZE];

off_t fsize(const char *path)
{
	struct stat st;
	stat(path, &st);
	return st.st_size;
}

int fcopy(const char *sourcePath, const char *destinationPath)
{
	// Get source file's size
	off_t srcsize = fsize(sourcePath);

	toncset(copyBuf, 0, COPY_BUF_SIZE);
	FILE* sourceFile = fopen(sourcePath, "rb");
	if (!sourceFile) {
		sassert(0, "invalid source file");
		return 1;
	}

	FILE* destinationFile = fopen(destinationPath, "wb");
	if (!destinationFile) {
		fclose(sourceFile);
		sassert(0, "couldnt open destfile");
		return 1;
	}

	off_t offset = 0;
	while (1)
	{
		// Copy file to destination path
		size_t numr = fread(copyBuf, 1, COPY_BUF_SIZE, sourceFile);
		fwrite(copyBuf, 1, numr, destinationFile);
		offset += COPY_BUF_SIZE;

		if (offset >= srcsize) {
			fflush(destinationFile);
			fclose(destinationFile);
			fclose(sourceFile);
			return 0;
		}
	}
	sassert(0, "wtf");
	// Should not get here...
	return 1;
}

int fsizeincrease(const char *sourcePath, const char *tempPath, size_t newsize)
{
	off_t srcsize = fsize(sourcePath);

    FILE* sourceFile = fopen(sourcePath, "rb");
    if (!sourceFile) {
		return 1;
	}

    FILE* destinationFile = fopen(tempPath, "wb");
	if (!destinationFile) {
		fclose(sourceFile);
		return 1;
	}

	off_t offset = 0;
	int numr;
	while (1)
	{
		// Copy file to destination path
		if (offset > srcsize) {
			numr = COPY_BUF_SIZE;
			toncset(copyBuf, 0, COPY_BUF_SIZE);
		} else {
			numr = fread(copyBuf, 1, COPY_BUF_SIZE, sourceFile);
		}
		fwrite(copyBuf, 1, numr, destinationFile);
		offset += COPY_BUF_SIZE;

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
