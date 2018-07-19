/*
	nitrofs.h - eris's wai ossum nitro filesystem device driver header  
		Based on information found at http://frangoassado.org/ds/rom_spec.txt and from the #dsdev ppls
		Kallisti (K) 2008-01-26 All rights reversed.

	2008-05-19  v0.2 - New And Improved!! :DDD
		* fix'd the fseek SEEK_CUR issue (my fseek funct should not have returned a value :/)
		* also thx to wintermute's input realized:
			* if you dont give ndstool the -o wifilogo.bmp option it will run on emulators in gba mode
			* you then dont need the gba's LOADEROFFSET, so it was set to 0x000

	2008-05-21  v0.3 - newer and more improved
		* fixed some issues with ftell() (again was fseek's fault u_u;;)
		* fixed possible error in detecting sc.gba files when using dldi
		* readded support for .gba files in addition to .nds emu
		* added stat() support for completedness :)

	2008-05-30  v0.5.Turbo - major speed improvement
		* This version uses a single filehandle to access the .nds file when not in GBA mode
		  improving the speed it takes to open a .nds file by around 106ms. This is great for 
		  situations requiring reading alot of seperate small files. However it does take a little
		  bit longer when reading from multiple files simultainously 
		  (around 122ms over 10,327 0x100 byte reads between 2 files).
	2008-06-09  
		* Fixed bug with SEEK_END where it wouldnt utilize the submitted position.. 
		  (now can fseek(f,-128,SEEK_END) to read from end of file :D)

	2008-06-18 v0.6.Turbo - . and .. :D
		* Today i have added full "." and ".." support.
		  dirnext() will return . and .. first, and all relevent operations will 
		  support . and .. in pathnames. 
*/

#ifndef NITROFS_H
#define NITROFS_H

#include <sys/dir.h>
#include <sys/iosupport.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int nitroFSInit(const char *ndsfile);
DIR_ITER* nitroFSDirOpen(struct _reent *r, DIR_ITER *dirState, const char *path);
int nitroDirReset(struct _reent *r, DIR_ITER *dirState);
int nitroFSDirNext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st);
int nitroFSDirClose(struct _reent *r, DIR_ITER *dirState);
int nitroFSOpen(struct _reent *r, void *fileStruct, const char *path,int flags,int mode);
int nitroFSClose(struct _reent *r,int fd);
int nitroFSRead(struct _reent *r,int fd,char *ptr,int len);
int nitroFSSeek(struct _reent *r,int fd,int pos,int dir);
int nitroFSFstat(struct _reent *r,int fd,struct stat *st);
int nitroFSstat(struct _reent *r,const char *file,struct stat *st);
int nitroFSChdir(struct _reent *r,const char *name);
#define LOADERSTR	"PASS"	//look for this
#define LOADERSTROFFSET 0xac
#define LOADEROFFSET 	0x0200
#define FNTOFFSET 	0x40
#define FATOFFSET 	0x48

#define NITRONAMELENMAX 0x80	//max file name is 127 +1 for zero byte :D
#define NITROMAXPATHLEN	0x100	//256 bytes enuff?

#define NITROROOT 	0xf000	//root entry_file_id
#define NITRODIRMASK	0x0fff	//remove leading 0xf

#define NITROISDIR 0x80	//mask to indicate this name entry is a dir, other 7 bits = name length

//Directory filename subtable entry structure
struct ROM_FNTDir {
	u32 	entry_start;
	u16	entry_file_id;
	u16	parent_id;
};

//Yo, dis table is fat (describes the structures
struct ROM_FAT {
	u32	top;	//start of file in rom image
	u32	bottom;	//end of file in rom image
};

struct nitroFSStruct {
	unsigned int	pos;	//where in the file am i?
	unsigned int 	start;	//where in the rom this file starts
	unsigned int 	end;	//where in the rom this file ends
};

struct nitroDIRStruct {
	unsigned int	pos;	//where in the file am i?
	unsigned int 	namepos;	//ptr to next name to lookup in list
	struct ROM_FAT  romfat;
	u16		entry_id;	//which entry this is (for files only) incremented with each new file in dir?
	u16		dir_id;		//which directory entry this is.. used ofc for dirs only
	u16		cur_dir_id;	//which directory entry we are using
	u16		parent_id;	//who is the parent of the current directory (this can be used to easily ../ )
	u8		spc;		//system path count.. used by dirnext, when 0=./ 1=../ >=2 actual dirs
};

#ifdef __cplusplus
}
#endif

#endif
