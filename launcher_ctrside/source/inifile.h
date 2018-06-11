/*
    inifile.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009-2010 yellow wood goblin

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

#ifndef _INIFILE_H_
#define _INIFILE_H_

#include <string>
#include <vector>
#include <map>

class CIniFile
{
  public:
    CIniFile();
    explicit CIniFile(const std::string& filename);
    virtual ~CIniFile();

  public:
    bool LoadIniFile(const std::string& FileName);
    bool SaveIniFile(const std::string& FileName);
    bool SaveIniFileModified(const std::string& FileName);

    std::string GetString(const std::string& Section,const std::string& Item,const std::string& DefaultValue);
    void SetString(const std::string& Section,const std::string& Item,const std::string& Value);
    int GetInt(const std::string& Section,const std::string& Item,int DefaultValue);
    void SetInt(const std::string& Section,const std::string& Item,int Value);
    void GetStringVector(const std::string& Section,const std::string& Item,std::vector<std::string>& strings,char delimiter=',');
    void SetStringVector(const std::string& Section,const std::string& Item,std::vector<std::string>& strings,char delimiter=',');
  protected:
    std::string m_sFileName;
    typedef std::vector<std::string> cStringArray;
    cStringArray m_FileContainer;
    bool m_bLastResult;
    bool m_bModified;
    bool m_bReadOnly;
    typedef std::map<std::string,size_t> cSectionCache;
    cSectionCache m_Cache;

    bool InsertLine(size_t line,const std::string& str);
    bool ReplaceLine(size_t line,const std::string& str);

    void SetFileString(const std::string& Section,const std::string& Item,const std::string& Value);
    std::string GetFileString(const std::string& Section,const std::string& Item);

    std::string GetString(const std::string& Section,const std::string& Item);
    int GetInt(const std::string& Section,const std::string& Item);
};

#endif // _INIFILE_H_

