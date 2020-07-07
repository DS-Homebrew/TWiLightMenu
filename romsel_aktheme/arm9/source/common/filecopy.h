#ifndef __ROMSEL_COMMON_FILE_COPY_H__
#define __ROMSEL_COMMON_FILE_COPY_H__

#ifdef __cplusplus
extern "C" {
#endif
int fcopy(const char *sourcePath, const char *destinationPath);
int fsizeincrease(const char *sourcePath, const char *tempPath, size_t newsize);
off_t fsize(const char *path);
#ifdef __cplusplus
}
#endif

#endif /* __ROMSEL_COMMON_FILE_COPY_H__ */