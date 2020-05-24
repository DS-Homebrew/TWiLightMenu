#ifndef FILE_COPY
#define FILE_COPY

int fcopy(const char *sourcePath, const char *destinationPath);
int fsizeincrease(const char *sourcePath, const char *tempPath, size_t newsize);

#endif // FILE_COPY