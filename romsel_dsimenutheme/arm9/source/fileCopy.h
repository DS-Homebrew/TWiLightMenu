#ifndef FILE_COPY
#define FILE_COPY

off_t getFileSize(const char *fileName);
int fcopy(const char *sourcePath, const char *destinationPath);

#endif // FILE_COPY