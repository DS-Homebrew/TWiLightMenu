/*
	nitrofs.c - eris's wai ossum nitro filesystem device driver
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

	2008-05-22  v0.3.1 - slight update
		* again fixed fseek(), this time SEEK_END oddly i kinda forgot about it >_> sry
		* also went ahead and inlined the functions, makes slight proformance improvement

	2008-05-26  v0.4 - added chdir
		* added proper chdir functionality

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

	2009-05-10 v0.7.Turbo - small changes  @_@?!

	2009-08-08 v0.8.Turbo - fix fix fix
		* fixed problem with some cards where the header would be loaded to GBA ram even if running 
                  in NDS mode causing nitroFSInit() to think it was a valid GBA cart header and attempt to 
           	  read from GBA SLOT instead of SLOT 1. Fixed this by making it check that filename is not NULL
 		  and then to try FAT/SLOT1 first. The NULL option allows forcing nitroFS to use gba.
    
    2018-09-05 v0.9 - modernize devoptab (by RonnChyran)
        * Updated for libsysbase change in devkitARM r46 and above. 
    
    2020-08-20 v0.10 - modernize GBA SLOT support (by RocketRobz)
        * Updated GBA SLOT detection to check for game code and header CRC. 

*/

#include <string.h>
#include <errno.h>
#include <nds.h>
#include "common/nitrofs.h"
#include "common/tonccpy.h"

#define __itcm __attribute__((section(".itcm")))

//Globals!
u32 fntOffset;   //offset to start of filename table
u32 fatOffset;   //offset to start of file alloc table
bool hasLoader;  //single global nds filehandle (is null if not in dldi/fat mode)
u16 chdirpathid; //default dir path id...
FILE *ndsFile;
off_t ndsFileLastpos; //Used to determine need to fseek or not

devoptab_t nitroFSdevoptab = {
    "nitro",                       //	const char *name;
    sizeof(struct nitroFSStruct),  //	int	structSize;
    &nitroFSOpen,                  //	int (*open_r)(struct _reent *r, void *fileStruct, const char *path,int flags,int mode);
    &nitroFSClose,                 //	int (*close_r)(struct _reent *r,void* fd);
    NULL,                          //	int (*write_r)(struct _reent *r,void* fd,const char *ptr,int len);
    &nitroFSRead,                  //	int (*read_r)(struct _reent *r,void* fd,char *ptr,int len);
    &nitroFSSeek,                  //	int (*seek_r)(struct _reent *r,void* fd,int pos,int dir);
    &nitroFSFstat,                 //	int (*fstat_r)(struct _reent *r,void* fd,struct stat *st);
    &nitroFSstat,                  //	int (*stat_r)(struct _reent *r,const char *file,struct stat *st);
    NULL,                          //	int (*link_r)(struct _reent *r,const char *existing, const char  *newLink);
    NULL,                          //	int (*unlink_r)(struct _reent *r,const char *name);
    &nitroFSChdir,                 //	int (*chdir_r)(struct _reent *r,const char *name);
    NULL,                          //	int (*rename_r) (struct _reent *r, const char *oldName, const char *newName);
    NULL,                          //	int (*mkdir_r) (struct _reent *r, const char *path, int mode);
    sizeof(struct nitroDIRStruct), //	int dirStateSize;
    &nitroFSDirOpen,               //	DIR_ITER* (*diropen_r)(struct _reent *r, DIR_ITER *dirState, const char *path);
    &nitroDirReset,                //	int (*dirreset_r)(struct _reent *r, DIR_ITER *dirState);
    &nitroFSDirNext,               //	int (*dirnext_r)(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *filestat);
    &nitroFSDirClose               //	int (*dirclose_r)(struct _reent *r, DIR_ITER *dirState);

};

//K, i decided to inline these, improves speed slightly..
//these 2 'sub' functions deal with actually reading from either gba rom or .nds file :)
//what i rly rly rly wanna know is how an actual nds cart reads from itself, but it seems no one can tell me ~_~
//so, instead we have this weird weird haxy try gbaslot then try dldi method. If i (or you!!) ever do figure out
//how to read the proper way can replace these 4 functions and everything should work normally :)

//reads from rom image either gba rom or dldi
inline ssize_t nitroSubRead(off_t *npos, void *ptr, size_t len)
{
    if (ndsFile != NULL)
    { //read from ndsfile
        if (ndsFileLastpos != *npos)
            fseek(ndsFile, *npos, SEEK_SET); //if we need to, move! (might want to verify this succeed)
        len = fread(ptr, 1, len, ndsFile);
    }
    else
    {                                             //reading from gbarom
        tonccpy(ptr, *npos + (void *)GBAROM, len); //len isnt checked here because other checks exist in the callers (hopefully)
    }
    if (len > 0)
        *npos += len;
    ndsFileLastpos = *npos; //save the current file nds pos
    return (len);
}

//seek around
inline void nitroSubSeek(off_t *npos, int pos, int dir)
{
    if ((dir == SEEK_SET) || (dir == SEEK_END)) //otherwise just set the pos :)
        *npos = pos;
    else if (dir == SEEK_CUR)
        *npos += pos; //see ez!
}

//Figure out if its gba or ds, setup stuff
int __itcm
nitroFSInit(const char *ndsfile)
{
    off_t pos = 0;
    char romstr[0x10];
    chdirpathid = NITROROOT;
    ndsFileLastpos = 0;
    ndsFile = NULL;
	if ((strncmp((const char *)0x02FFFC38, __NDSHeader->gameCode, 4) == 0) && (*(u16*)0x02FFFC36 == __NDSHeader->headerCRC16))
	{
		sysSetCartOwner (BUS_OWNER_ARM9); //give us gba slot ownership
		// We has gba rahm
		fntOffset = ((u32) * (u32 *)(((const char *)GBAROM) + FNTOFFSET));
		fatOffset = ((u32) * (u32 *)(((const char *)GBAROM) + FATOFFSET));
		hasLoader = false;
		AddDevice(&nitroFSdevoptab);
		return (1);
	}
    if (ndsfile != NULL)
    {
        if ((ndsFile = fopen(ndsfile, "rb")))
        {
            nitroSubRead(&pos, romstr, strlen(LOADERSTR));
            if (strncmp(romstr, LOADERSTR, strlen(LOADERSTR)) == 0)
            {
                nitroSubSeek(&pos, LOADEROFFSET + FNTOFFSET, SEEK_SET);
                nitroSubRead(&pos, &fntOffset, sizeof(fntOffset));
                nitroSubSeek(&pos, LOADEROFFSET + FATOFFSET, SEEK_SET);
                nitroSubRead(&pos, &fatOffset, sizeof(fatOffset));
                fatOffset += LOADEROFFSET;
                fntOffset += LOADEROFFSET;
                hasLoader = true;
            }
            else
            {
                nitroSubSeek(&pos, FNTOFFSET, SEEK_SET);
                nitroSubRead(&pos, &fntOffset, sizeof(fntOffset));
                nitroSubSeek(&pos, FATOFFSET, SEEK_SET);
                nitroSubRead(&pos, &fatOffset, sizeof(fatOffset));
                hasLoader = false;
            }
            setvbuf(ndsFile, NULL, _IONBF, 0); //we dont need double buffs u_u
            AddDevice(&nitroFSdevoptab);
            return (1);
        }
    }
    return (0);
}

//Directory functs
DIR_ITER *nitroFSDirOpen(struct _reent *r, DIR_ITER *dirState, const char *path)
{
    struct nitroDIRStruct *dirStruct = (struct nitroDIRStruct *)dirState->dirStruct; //this makes it lots easier!
    struct stat st;
    char dirname[NITRONAMELENMAX];
    char *cptr;
    char mydirpath[NITROMAXPATHLEN]; //to hold copy of path string
    char *dirpath = mydirpath;
    bool pathfound;
    if ((cptr = strchr(path, ':')))
        path = cptr + 1;                           //move path past any device names (if it was nixy style wouldnt need this step >_>)
    strncpy(dirpath, path, sizeof(mydirpath) - 1); //copy the string (as im gonna mutalate it)
    dirStruct->pos = 0;
    if (*dirpath == '/')                   //if first character is '/' use absolute root path plz
        dirStruct->cur_dir_id = NITROROOT; //first root dir
    else
        dirStruct->cur_dir_id = chdirpathid; //else use chdirpath
    nitroDirReset(r, dirState);              //set dir to current path
    do
    {
        while ((cptr = strchr(dirpath, '/')) == dirpath)
        {
            dirpath++; //move past any leading / or // together
        }
        if (cptr)
            *cptr = 0; //erase /
        if (*dirpath == 0)
        {                     //are we at the end of the path string?? if so there is nothing to search for we're already here !
            pathfound = true; //mostly this handles searches for root or /  or no path specified cases
            break;
        }
        pathfound = false;
        while (nitroFSDirNext(r, dirState, dirname, &st) == 0)
        {
            if ((st.st_mode == S_IFDIR) && !(strcmp(dirname, dirpath)))
            {                                              //if its a directory and name matches dirpath
                dirStruct->cur_dir_id = dirStruct->dir_id; //move us to the next dir in tree
                nitroDirReset(r, dirState);                //set dir to current path we just found...
                pathfound = true;
                break;
            }
        };
        if (!pathfound)
            break;
        dirpath = cptr + 1; //move to right after last / we found
    } while (cptr);         // go till after the last /
    if (pathfound)
    {
        return (dirState);
    }
    else
    {
        r->_errno = ENOENT;
        return (NULL);
    }
}

int nitroFSDirClose(struct _reent *r, DIR_ITER *dirState)
{
    return (0);
}

/*Consts containing relative system path strings*/
const char *syspaths[2] = {
    ".",
    ".."};

//reset dir to start of entry selected by dirStruct->cur_dir_id which should be set in dirOpen okai?!
int nitroDirReset(struct _reent *r, DIR_ITER *dirState)
{
    struct nitroDIRStruct *dirStruct = (struct nitroDIRStruct *)dirState->dirStruct; //this makes it lots easier!
    struct ROM_FNTDir dirsubtable;
    off_t *pos = &dirStruct->pos;
    nitroSubSeek(pos, fntOffset + ((dirStruct->cur_dir_id & NITRODIRMASK) * sizeof(struct ROM_FNTDir)), SEEK_SET);
    nitroSubRead(pos, &dirsubtable, sizeof(dirsubtable));
    dirStruct->namepos = dirsubtable.entry_start;    //set namepos to first entry in this dir's table
    dirStruct->entry_id = dirsubtable.entry_file_id; //get number of first file ID in this branch
    dirStruct->parent_id = dirsubtable.parent_id;    //save parent ID in case we wanna add ../ functionality
    dirStruct->spc = 0;                              //system path counter, first two dirnext's deliver . and ..
    return (0);
}

int nitroFSDirNext(struct _reent *r, DIR_ITER *dirState, char *filename, struct stat *st)
{
    unsigned char next;
    struct nitroDIRStruct *dirStruct = (struct nitroDIRStruct *)dirState->dirStruct; //this makes it lots easier!
    off_t *pos = &dirStruct->pos;
    if (dirStruct->spc <= 1)
    {
        if (st)
            st->st_mode = S_IFDIR;
        if ((dirStruct->spc == 0) || (dirStruct->cur_dir_id == NITROROOT))
        { // "." or its already root (no parent)
            dirStruct->dir_id = dirStruct->cur_dir_id;
        }
        else
        { // ".."
            dirStruct->dir_id = dirStruct->parent_id;
        }
        strcpy(filename, syspaths[dirStruct->spc++]);
        return (0);
    }
    nitroSubSeek(pos, fntOffset + dirStruct->namepos, SEEK_SET);
    nitroSubRead(pos, &next, sizeof(next));
    // next: high bit 0x80 = entry isdir.. other 7 bits r size, the 16 bits following name are dir's entryid (starts with f000)
    //  00 = endoftable //
    if (next)
    {
        if (next & NITROISDIR)
        {
            if (st)
                st->st_mode = S_IFDIR;
            next &= NITROISDIR ^ 0xff; //invert bits and mask off 0x80
            nitroSubRead(pos, filename, next);
            nitroSubRead(&dirStruct->pos, &dirStruct->dir_id, sizeof(dirStruct->dir_id)); //read the dir_id
                                                                                          //grr cant get the struct member size?, just wanna test it so moving on...
                                                                                          //			nitroSubRead(pos,&dirStruct->dir_id,sizeof(u16)); //read the dir_id
            dirStruct->namepos += next + sizeof(u16) + 1;                                 //now we points to next one plus dir_id size:D
        }
        else
        {
            if (st)
                st->st_mode = 0;
            nitroSubRead(pos, filename, next);
            dirStruct->namepos += next + 1; //now we points to next one :D
            //read file info to get filesize (and for fileopen)
            nitroSubSeek(pos, fatOffset + (dirStruct->entry_id * sizeof(struct ROM_FAT)), SEEK_SET);
            nitroSubRead(pos, &dirStruct->romfat, sizeof(dirStruct->romfat)); //retrieve romfat entry (contains filestart and end positions)
            dirStruct->entry_id++;                                            //advance ROM_FNTStrFile ptr
            if (st)
                st->st_size = dirStruct->romfat.bottom - dirStruct->romfat.top; //calculate filesize
        }
        filename[(int)next] = 0; //zero last char
        return (0);
    }
    else
    {
        r->_errno = EIO;
        return (-1);
    }
}

//fs functs
int nitroFSOpen(struct _reent *r, void *fileStruct, const char *path, int flags, int mode)
{
    struct nitroFSStruct *fatStruct = (struct nitroFSStruct *)fileStruct;
    struct nitroDIRStruct dirStruct;
    DIR_ITER dirState;
    dirState.dirStruct = &dirStruct; //create a temp dirstruct
    struct _reent dre;
    struct stat st;                     //all these are just used for reading the dir ~_~
    char dirfilename[NITROMAXPATHLEN];  // to hold a full path (i tried to avoid using so much stack but blah :/)
    char *filename;                     // to hold filename
    char *cptr;                         //used to string searching and manipulation
    cptr = (char *)path + strlen(path); //find the end...
    filename = NULL;
    do
    {
        if ((*cptr == '/') || (*cptr == ':'))
        { // split at either / or : (whichever comes first form the end!)
            cptr++;
            strncpy(dirfilename, path, cptr - path); //copy string up till and including/ or : zero rest
            dirfilename[cptr - path] = 0;            //it seems strncpy doesnt always zero?!
            filename = cptr;                         //filename = now remainder of string
            break;
        }
    } while (cptr-- != path); //search till start
    if (!filename)
    {                            //we didnt find a / or : ? shouldnt realyl happen but if it does...
        filename = (char *)path; //filename = complete path
        dirfilename[0] = 0;      //make directory path ""
    }
    if (nitroFSDirOpen(&dre, &dirState, dirfilename))
    {
        fatStruct->start = 0;
        while (nitroFSDirNext(&dre, &dirState, dirfilename, &st) == 0)
        {
            if (!(st.st_mode & S_IFDIR) && (strcmp(dirfilename, filename) == 0))
            { //Found the *file* youre looking for!!
                fatStruct->start = dirStruct.romfat.top;
                fatStruct->end = dirStruct.romfat.bottom;
                if (hasLoader)
                {
                    fatStruct->start += LOADEROFFSET;
                    fatStruct->end += LOADEROFFSET;
                }
                break;
            }
        }
        if (fatStruct->start)
        {
            nitroSubSeek(&fatStruct->pos, fatStruct->start, SEEK_SET); //seek to start of file
            return (0);                                                //woot!
        }
        nitroFSDirClose(&dre, &dirState);
    }
    if (r->_errno == 0)
    {
        r->_errno = ENOENT;
    }
    return (-1); //teh fail
}

int nitroFSClose(struct _reent *r, void* fd)
{
    return (0);
}

ssize_t nitroFSRead(struct _reent *r, void* fd, char *ptr, size_t len)
{
    struct nitroFSStruct *fatStruct = (struct nitroFSStruct *)fd;
    off_t *npos = &fatStruct->pos;
    if (*npos + len > fatStruct->end)
        len = fatStruct->end - *npos; //dont let us read past the end plz!
    if (*npos > fatStruct->end)
        return (0); //hit eof
    return (nitroSubRead(npos, ptr, len));
}

off_t nitroFSSeek(struct _reent *r, void* fd, off_t pos, int dir)
{
    //need check for eof here...
    struct nitroFSStruct *fatStruct = (struct nitroFSStruct *)fd;
    off_t *npos = &fatStruct->pos;
    if (dir == SEEK_SET)
        pos += fatStruct->start; //add start from .nds file offset
    else if (dir == SEEK_END)
        pos += fatStruct->end; //set start to end of file (useless?)
    if (pos > fatStruct->end)
        return (-1); //dont let us read past the end plz!
    nitroSubSeek(npos, pos, dir);
    return (*npos - fatStruct->start);
}

int nitroFSFstat(struct _reent *r, void* fd, struct stat *st)
{
    struct nitroFSStruct *fatStruct = (struct nitroFSStruct *)fd;
    st->st_size = fatStruct->end - fatStruct->start;
    return (0);
}

int nitroFSstat(struct _reent *r, const char *file, struct stat *st)
{
    struct nitroFSStruct fatStruct;
    struct nitroDIRStruct dirStruct;
    DIR_ITER dirState;

    if (nitroFSOpen(NULL, &fatStruct, file, 0, 0) >= 0)
    {
        st->st_mode = S_IFREG;
        st->st_size = fatStruct.end - fatStruct.start;
        return (0);
    }

    dirState.dirStruct = &dirStruct;
    if ((nitroFSDirOpen(r, &dirState, file) != NULL))
    {

        st->st_mode = S_IFDIR;
        nitroFSDirClose(r, &dirState);
        return (0);
    }
    r->_errno = ENOENT;
    return (-1);
}

int nitroFSChdir(struct _reent *r, const char *name)
{
    struct nitroDIRStruct dirStruct;
    DIR_ITER dirState;
    dirState.dirStruct = &dirStruct;
    if ((name != NULL) && (nitroFSDirOpen(r, &dirState, name) != NULL))
    {
        chdirpathid = dirStruct.cur_dir_id;
        nitroFSDirClose(r, &dirState);
        return (0);
    }
    else
    {
        r->_errno = ENOENT;
        return (-1);
    }
}