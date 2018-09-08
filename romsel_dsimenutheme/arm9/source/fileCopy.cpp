#include <nds.h>
#include <stdio.h>

u32 copyBuf[2];

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
		return -1;
	}

    FILE* destinationFile = fopen(destinationPath, "wb");
	//if (destinationFile) {
		fseek(destinationFile, 0, SEEK_SET);
	/*} else {
		fclose(sourceFile);
		fclose(destinationFile);
		return -1;
	}*/

	off_t offset = 0;
	int numr;
	while (1)
	{
		// Copy file to destination path
		numr = fread(copyBuf, 2, 1, sourceFile);
		fwrite(copyBuf, 2, numr, destinationFile);
		offset += 1;

		if (offset > fsize) {
			fclose(sourceFile);
			fclose(destinationFile);
			return 1;
			break;
		}
	}

	return -1;
}
