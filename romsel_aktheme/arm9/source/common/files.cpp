/*
    files.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <nds.h>
#include <string>
#include "files.h"
//#include "dbgtool.h"
// #include <elm.h>
#include <dirent.h>
#include <fat.h>
#include <sys/iosupport.h>
#include "ui/msgbox.h"
#include "ui/progresswnd.h"
#include <errno.h>
#include <cstdio>
#include <unistd.h>
#include "language.h"
#include "time/datetime.h"
#include "dsrom.h"
// #include "favorites.h"
#include <sys/statvfs.h>

// #define USE_OPEN
#ifdef USE_OPEN
#include <fcntl.h>
#endif

using namespace akui;

static SRC_FILE_MODE _srcFileMode = SFM_COPY;
static std::string _srcFilename = "";

bool loadFile(void * buffer, const std::string & filename, size_t offset, size_t & readCount)
{
    if ("" == filename)
        return false;

    if (NULL == buffer) {
        dbg_printf("invalid buffer pointer\n");
        struct stat st;
        stat(filename.c_str(), &st);
        readCount = st.st_size;
        return false;
    }

    FILE * f = fopen(filename.c_str(), "rb");
    if (NULL == f) {
        dbg_printf("file does not exist\n");
        readCount = 0;
        return false;
    }

    fseek(f, 0, SEEK_END);
    int fileSize = ftell(f);

    if (-1 == fileSize) {
        fclose(f);
        readCount = 0;
        return false;
    }

    fseek(f, offset, SEEK_SET);
    size_t readed = fread(buffer, 1, fileSize, f);
    fclose(f);

    readCount = readed;
    if (readed != (size_t)fileSize-offset) {
        dbg_printf("fread fail: %d/%d\n", readed, fileSize);
        readCount = 0;
        return false;
    }

    return true;
}

int getFileSize(const std::string & filename)
{
    if ("" == filename)
        return -1;

    struct stat st;
    if (-1 == stat(filename.c_str(), &st)) {
        return -1;
    }
    return st.st_size;
}



//---------------------------------------------------------------------------------//
bool stopCopying = false;
bool copyingFile = false;

bool copyFile(const std::string & srcFilename, const std::string & destFilename, bool silently, size_t copyLength)
{
    dbg_printf("copy %s to %s\n", srcFilename.c_str(), destFilename.c_str());
    struct stat srcSt;
    if (0 != stat(srcFilename.c_str(), &srcSt)) {
        messageBox(NULL, LANG("copy file error","title"), LANG("copy file error","text"), MB_OK);
        return false;
    }

    u64 total = 0;
    u64 used = 0;
    u64 freeSpace = 0;

    std::string destDiskName = destFilename.substr(0, 6);
    if (destDiskName != "fat0:/" && destDiskName != "fat1:/") {
		destDiskName = destFilename.substr(0, 5);
		if (destDiskName != "fat:/") {
			destDiskName = destFilename.substr(0, 4);
			if (destDiskName != "sd:/") {
				return false;
			}
		}
	}

    if (!getDiskSpaceInfo(destDiskName, total, used, freeSpace)) {
        messageBox(NULL, LANG("no free space","title"), LANG("no free space","text"), MB_OK);
        return false;
    }
	
	dbg_printf("copyLength", copyLength);

    if (0 == copyLength || copyLength > (size_t)srcSt.st_size)
        copyLength = srcSt.st_size;
		
	dbg_printf("copyLength %X", copyLength);
		
	dbg_printf("freeSpace %X", freeSpace);

    if (freeSpace < copyLength) {
        messageBox(NULL, LANG("no free space","title"), LANG("no free space","text"), MB_OK);
        return false;
    }

    if (!silently) {
        struct stat destSt;
        if (0 == stat(destFilename.c_str(), &destSt)) {
            if (!(destSt.st_mode & S_IFDIR)) {
                u32 ret = messageBox(NULL, LANG("copy file exists","title"),
                    LANG("copy file exists","text"), MB_YES | MB_NO);
                if (ret != ID_YES)
                    return false;
            } else {
                messageBox(NULL, LANG("copy dest is directory","title"),
                    LANG("copy dest is directory","text"), MB_CANCEL);
                return false;
            }
        }
    }

    u32 copyBufferSize=ms().CopyBufferSize();
    u8* copyBuffer=new(std::nothrow)u8[copyBufferSize];
    if (copyBuffer==NULL)
    {
      messageBox(NULL,LANG("ram allocation","title"),LANG("ram allocation","memory allocation error"),MB_OK);
      return false;
    }

    std::string tempText = LANG("progress window", "processing copy");
    std::string copyTip = formatString(tempText.c_str(), (char)0x02);
    progressWnd().setTipText(copyTip);
    progressWnd().show();
    progressWnd().setPercent(0);
    stopCopying = false;
    copyingFile = true;

#ifdef USE_OPEN
    int wf=open(destFilename.c_str(),O_WRONLY|O_CREAT|O_TRUNC);
    int rf=open(srcFilename.c_str(),O_RDONLY);
#else
    FILE * wf = fopen(destFilename.c_str(), "wb");
    FILE * rf = fopen(srcFilename.c_str(), "rb");
#endif
    u8 percent = 0;

    dbg_printf("start: %s", datetime().getTimeString().c_str());

    u32 writeCount = copyLength / copyBufferSize;
    if (copyLength % copyBufferSize)
        writeCount++;
    dbg_printf("write count %d\n", writeCount);

    u32 remain = copyLength;

    for (size_t i = 0; i < writeCount; ++i) {
        if (stopCopying) {
            copyingFile = false;
            u32 ret = messageBox(&progressWnd(), LANG("stop copying file","title"),
                LANG("stop copying file","text"), MB_YES | MB_NO);

            if (ID_YES == ret) {
#ifdef USE_OPEN
                close(rf);
                close(wf);
#else
                fclose(rf);
                fclose(wf);
#endif
                progressWnd().hide();
                copyingFile = false;
                delete[] copyBuffer;
                return false;
            }
            copyingFile = true;
            stopCopying = false;
        }

        u32 toRead = remain > copyBufferSize ? copyBufferSize : remain;
#ifdef USE_OPEN
        ssize_t readed=read(rf,copyBuffer,toRead);
        ssize_t written=write(wf,copyBuffer,readed);
#else
        u32 readed = fread(copyBuffer, 1, toRead, rf);
        u32 written = fwrite(copyBuffer, 1, (int)readed, wf);
#endif
        if (written != readed) {
            dbg_printf("err %d\n", errno);
            dbg_printf("COPY FILE ERROR! %d/%d\n", readed, written);
            // todo: judge error types in errno
#ifdef USE_OPEN
            close(rf);
            close(wf);
#else
            fclose(rf);
            fclose(wf);
#endif
            progressWnd().hide();
            copyingFile = false;
            delete[] copyBuffer;
            messageBox(NULL, LANG("no free space","title"), LANG("no free space","text"), MB_OK);
            return false;
        }
        remain -= written;
        percent = i * 100 / writeCount;
        progressWnd().setPercent(percent);
    }
#ifdef USE_OPEN
    close(rf);
    close(wf);
#else
    fclose(rf);
    fclose(wf);
#endif
    progressWnd().hide();
    copyingFile = false;
    delete[] copyBuffer;

    dbg_printf("finish: %s", datetime().getTimeString().c_str());
    return true;
}

bool renameFile(const std::string & oldName, const std::string & newName)
{
    if ("" == oldName || "" == newName)
        return false;

    struct stat destSt;
    if (0 == stat(newName.c_str(), &destSt)) {
        if (!(destSt.st_mode & S_IFDIR)) {
            u32 ret = messageBox(NULL, LANG("copy file exists","title"),
                LANG("copy file exists","text"), MB_YES | MB_NO);
            if (ret != ID_YES)
                return false;
        } else {
            messageBox(NULL, LANG("move dest is directory","title"),
                LANG("move dest is directory","text"), MB_CANCEL);
            return false;
        }
    }

    if (0 != rename(oldName.c_str(), newName.c_str())) {
        if (EEXIST == errno || EXDEV == errno) {
            return (copyFile(oldName, newName, true));
        }
        return false;
    }

    return true;
}

bool deleteFile(const std::string & filename)
{
    if ("" == filename)
        return false;

    struct stat destSt;
    if (0 != stat(filename.c_str(), &destSt)) {
        return false;
    }

    std::string confirmText = LANG("confirm delete file","text");
    std::string showname,realname;
    if ('/' == filename[filename.size()-1])
        showname = filename.substr(0, filename.size() - 1);
    else
        showname = filename;
    realname = showname;

    size_t slashPos = showname.find_last_of('/');
    if (showname.npos != slashPos)
        showname = showname.substr(slashPos + 1);

    confirmText = formatString(confirmText.c_str(), showname.c_str());
    u32 result = messageBox(NULL, LANG("confirm delete file","title"), confirmText.c_str(), MB_YES | MB_NO);
    if (result != ID_YES) {
        return false;
    }

    int ret = unlink(realname.c_str());
    if (0 != ret) {
        if (EACCES == errno) {
            messageBox(NULL, LANG("do not delete directory","title"), LANG("do not delete directory","text"), MB_OK);
        }
        return false;
    }
    // cFavorites::RemoveFromFavorites(filename);
    return true;
}

bool hideFile(const std::string & filename)
{
    if ("" == filename)
        return false;

    struct stat destSt;
    if (0 != stat(filename.c_str(), &destSt)) {
        return false;
    }

    bool isHidden = FAT_getAttr(filename.c_str()) & ATTR_HIDDEN;

    std::string confirmText = LANG("confirm hide file", isHidden ? "unhide" : "hide");
    std::string showname,realname;
    if ('/' == filename[filename.size()-1])
        showname = filename.substr(0, filename.size() - 1);
    else
        showname = filename;
    realname = showname;

    size_t slashPos = showname.find_last_of('/');
    if (showname.npos != slashPos)
        showname = showname.substr(slashPos + 1);

    confirmText = formatString(confirmText.c_str(), showname.c_str());
    u32 result = messageBox(NULL, LANG("confirm hide file","title"), confirmText.c_str(), MB_YES | MB_NO);
    if (result != ID_YES) {
        return false;
    }

    FAT_setAttr(filename.c_str(), FAT_getAttr(filename.c_str()) ^ ATTR_HIDDEN);

    return true;
}

void setSrcFile(const std::string & filename, SRC_FILE_MODE mode)
{
    _srcFilename = filename;
    _srcFileMode = mode;
}

const std::string& getSrcFile(void)
{
  return _srcFilename;
}

bool copyOrMoveFile(const std::string & destDir)
{
    if ("" == _srcFilename)
        return false;

    const char * pPath = _srcFilename.c_str();
    const char * pName = NULL;
    while (pPath < _srcFilename.c_str() + _srcFilename.size())
    {
        if ('/' == *pPath++)
            pName = pPath;
    }

    if (0 == *pName)
        return false;

    std::string destPath = destDir + pName;
    if (destPath == _srcFilename)
        return false;

    if (SFM_COPY == _srcFileMode)
    {
        u32 copyLength = 0;

        // if (ms().romTrim)    {
        //     std::string extName;
        //     size_t lastDotPos = _srcFilename.find_last_of('.');
        //     if (_srcFilename.npos != lastDotPos)
        //         extName = _srcFilename.substr(lastDotPos);
        //     else
        //         extName = "";
        //     for (size_t i = 0; i < extName.size(); ++i)
        //         extName[i] = tolower(extName[i]);

        //     if (".nds" == extName) {
        //         DSRomInfo info;
        //         info.MayBeDSRom(_srcFilename);
        //         if (info.isDSRom() && !info.isHomebrew()) {
        //             FILE * f = fopen(_srcFilename.c_str(), "rb");
        //             fseek(f, 0x80, SEEK_SET);
        //             fread(&copyLength, 1, 4, f);
        //             fclose(f);
        //             copyLength += 0x88;     // to keep RSA signature
        //         }
        //     }
        // }

        bool copyOK = copyFile(_srcFilename, destPath, false, copyLength);
        if (copyOK) {
            _srcFilename = "";
        }
        return copyOK;
    }

    if (SFM_CUT == _srcFileMode)
    {
        bool moveOK = renameFile(_srcFilename, destPath);
        if (moveOK) {
            // cFavorites::UpdateFavorites(_srcFilename,destPath);
            _srcFilename = "";
        }
        return moveOK;
    }
    return false;
}

// bool getDirSize(const std::string & path, bool includeSubdirs, u64 * dirSize)
// {
//     if ("" == path)
//         return false;

//     u64 size = 0;

//     std::string dirPath = path;
//     if (dirPath[dirPath.size()-1] != '/')
//         dirPath += "/";
//     if (dirPath.size() > PATH_MAX)
//         return false;

//     DIR *dir = NULL;
//     dir = opendir(dirPath.c_str());
//     if (dir == NULL) {
//         //dbg_printf("getDirSize couldn't open dir %s", path.c_str());
//         return false;
//     }

//     size = 0;
//     char filename[PATH_MAX];

//     struct stat stat_buf;
//     while (dirnext(dir, filename, &stat_buf) == 0) {

//         if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
//             continue;
//         }

//         //dbg_printf("getDirSize dir entry '%s'", path.c_str());
//         if (!(stat_buf.st_mode & S_IFDIR)) {
//             //dbg_printf("getDirSize add size %d for '%s'", (int)stat_buf.st_size, path.c_str());
//             size += (u64)stat_buf.st_size;
//         } else if (includeSubdirs) {
//             /* calculate the size recursively */
//             u64 subDirSize = 0;
//             bool succ = getDirSize(dirPath + filename, includeSubdirs, &subDirSize);
//             /* ignore failure in subdirs */
//             if (succ) {
//                 size += subDirSize;
//             }
//         }
//     }

//     dirclose(dir);
//     *dirSize = size;
//     return true;
// }

// static bool getDiskFromString(const std::string& diskName,u32& disk)
// {
//   if (tolower(diskName[0])=='f'&&tolower(diskName[1])=='a'&&tolower(diskName[2])=='t'&&(diskName[3]=='0'||diskName[3]=='1')&&diskName[4]==':')
//   {
//     disk=diskName[3]-'0';
//     return true;
//   }
//   return false;
// }

// static bool getDiskTotalSpace(u32 disk,u64& diskSpace)
// {
//   u32 clusters,clusterSize;
//   if (ELM_ClusterSizeFromDisk(disk,&clusterSize)&&ELM_ClustersFromDisk(disk,&clusters))
//   {
//     diskSpace=(u64)clusters*(u64)clusterSize;
//     return true;
//   }
//   return false;
// }

// static bool getDiskFreeSpace(u32 disk,u64& freeSpace)
// {
//   u32 clusters,clusterSize;
//   if (ELM_ClusterSizeFromDisk(disk,&clusterSize)&&ELM_FreeClustersFromDisk(disk,&clusters))
//   {
//     freeSpace=(u64)clusters*(u64)clusterSize;
//     return true;
//   }
//   return false;
// }

bool getDiskSpaceInfo(const std::string& diskName,u64& total,u64& used,u64& freeSpace)
{
  if (""==diskName) return false;
  
  struct statvfs info;
  
  if (statvfs(diskName.c_str(), &info) != 0) return false;
    
  total = info.f_frsize*info.f_blocks;
  freeSpace = info.f_frsize*info.f_bfree;
  used=total-freeSpace;
  return true;
}
