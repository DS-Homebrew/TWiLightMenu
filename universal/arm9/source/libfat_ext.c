#include <sys/iosupport.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>
#include <malloc.h>

static inline void* _FAT_mem_allocate (size_t size) {
	return malloc (size);
}

static inline void* _FAT_mem_align (size_t size) {
	return malloc (size);
}

static inline void _FAT_mem_free (void* mem) {
	free (mem);
}

#include "directory.h"
#include "partition.h"
#include "common/tonccpy.h"

//static int timesRan = 0;

static int fatGetAlias (const char* drive, const char* name, const char* nameEnd, char *alias) {
	devoptab_t *devops;
	PARTITION* partition;
	DIR_ENTRY entry;
	char *buf;
	int drivelen/*,namelen*/,i;

	if (!drive || !name)
		return -1;

	drivelen = strlen(drive);
	//namelen = strlen(name);
	buf=(char*)_FAT_mem_allocate(sizeof(char)*drivelen+2);	
	strcpy(buf,drive);

	if (drive[drivelen-1] == '/') {
		buf[drivelen-1]='\0';
		drivelen--;
	}

	if (drive[drivelen-1] != ':') {
		buf[drivelen]=':';
		buf[drivelen+1]='\0';
	}

	/*if (name[namelen-1] == '/') {
		name[namelen-1]='\0';
		namelen--;
	}*/

	devops = (devoptab_t*)GetDeviceOpTab(buf);

	for (i=0;buf[i]!='\0' && buf[i]!=':';i++);  
	if (!devops || strncasecmp(buf,devops->name,i)) {
		_FAT_mem_free(buf);
		return -1;
	}

	_FAT_mem_free(buf);

	partition = (PARTITION*)devops->deviceData;

	int resultLen = 0;

	if (_FAT_directory_entryFromPath(partition, &entry, name, nameEnd)) { 
		tonccpy(alias,&entry.entryData,8);
		int i = 0;
		if (strncmp((char*)entry.entryData+8, "   ", 3) != 0) {
			for (i = 0; i < 8; i++) {
				if (entry.entryData[i] == ' ') {
					break;
				}
			}
			alias[i] = '.';
			alias[i+1] = entry.entryData[8];
			alias[i+2] = entry.entryData[9];
			alias[i+3] = entry.entryData[10];
			alias[i+4] = '\0';
			resultLen = i+4;
		} else {
			for (i = 0; i <= 8; i++) {
				if (entry.entryData[i] == ' ') {
					alias[i] = _FAT_directory_isDirectory(&entry) ? '\x2F' : '\0';
					break;
				}
			}
			resultLen = i+1;
		}
		/*tonccpy((char*)0x02800000+(timesRan*0x20), entry.entryData, 0x20);
		tonccpy((char*)0x02800100+(timesRan*0x10), alias, 12);
		timesRan++;*/
	}

	return resultLen;
}

void fatGetAliasName (const char* drive, const char* name, char *alias) {
	fatGetAlias (drive, name, NULL, alias);
}

void fatGetAliasPath (const char* drive, const char* path, char *alias) {
	char dirBak[PATH_MAX];
	getcwd(dirBak, PATH_MAX);
	chdir(drive);

	char name[128];
	int len = (int)strlen(drive);
	int lfnLen = len;
	if (path[0] == '\x2F') {
		len = 1;
		lfnLen = len;
	}
	int nameEnd = 0;
	tonccpy(alias, drive, len);

	for (int i = len; i < 256; i++) {
		for (nameEnd = 0; nameEnd < 128; nameEnd++) {
			name[nameEnd] = path[i+nameEnd];
			if (path[i+nameEnd] == '\0' || path[i+nameEnd] == '\x2F' /*|| path[i+nameEnd] == '\x5C'*/) {
				name[nameEnd] = '\0';
				break;
			}
		}
		//tonccpy((char*)0x02800040+(timesRan*0x10), name, nameEnd);
		len += fatGetAlias (drive, path+lfnLen, path+lfnLen+nameEnd, alias+len);
		lfnLen += nameEnd+1;
		i += nameEnd;
		//timesRan++;
		if (path[i] == '\0') break;
		chdir(name);
	}

	chdir(dirBak);
}
