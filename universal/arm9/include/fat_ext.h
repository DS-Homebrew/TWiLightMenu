#ifndef _LIBFAT_EXT_H
#define _LIBFAT_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <nds/disc_io.h>

/*
Get Alias Name
*/
extern void fatGetAliasName (const char* drive, const char* name, char *alias);

/*
Get Alias Path
*/
extern void fatGetAliasPath (const char* drive, const char* path, char *alias);

#ifdef __cplusplus
}
#endif

#endif // _LIBFAT_EXT_H
