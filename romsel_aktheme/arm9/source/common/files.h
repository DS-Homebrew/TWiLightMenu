/*
    files.h
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

#ifndef _FILES_H_
#define _FILES_H_

#include <string>

bool loadFile( void * buffer, const std::string & filename, size_t offset, size_t & readCount );

// �����ļ���С��-1��ʾ�ļ�������
int getFileSize( const std::string & filename );

extern bool stopCopying;
extern bool copyingFile;
bool copyFile( const std::string & srcFilename, const std::string & destFilename, bool silently, size_t copyLength = 0 );

bool renameFile( const std::string & oldName, const std::string & newName );

bool deleteFile( const std::string & filename );

bool hideFile(const std::string & filename);

enum SRC_FILE_MODE
{
    SFM_COPY = 0,
    SFM_CUT = 1
};

void setSrcFile( const std::string & filename, SRC_FILE_MODE mode );

const std::string& getSrcFile(void);

bool copyOrMoveFile( const std::string & destDir );

bool getDirSize( const std::string & path, bool includeSubdirs, u64 * dirSize );

bool getDiskSpaceInfo( const std::string & diskName, u64 & total, u64 & used, u64 & freeSpace );

#endif//_FILES_H_
