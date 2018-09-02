#include "dsargv.h"

static bool freadLine(FILE *f, std::string &str)
{
    str.clear();
__read:
    char p = 0;

    size_t readed = fread(&p, 1, 1, f);
    if (0 == readed)
    {
        str = "";
        return false;
    }
    if ('\n' == p || '\r' == p)
    {
        str = "";
        return true;
    }

    while (p != '\n' && p != '\r' && readed)
    {
        str += p;
        readed = fread(&p, 1, 1, f);
    }

    if (str.empty() || "" == str)
    {
        goto __read;
    }

    return true;
}

static void trimString(std::string &str)
{
    size_t first = str.find_first_not_of(" \t"), last;
    if (first == str.npos)
    {
        str = "";
    }
    else
    {
        last = str.find_last_not_of(" \t");
        if (first > 0 || (last + 1) < str.length())
            str = str.substr(first, last - first + 1);
    }
}

ArgvFile::ArgvFile(const std::string &filename)
{

    FILE* fp = fopen(filename.c_str(), "rb");
    if (fp == NULL)
    {
        fclose(fp);
        return;
    }

    std::string strline("");
    while (freadLine(fp, strline))
    {
        trimString(strline);
        if (strline != "" && ';' != strline[0] && '/' != strline[0] && '!' != strline[0] && '#' != strline[0])
        {
            if (_launchPath.empty())
            {
                _launchPath = strline;
            }
            else
            {
                _launchArgs.emplace_back(strline);
            }
        }
    }
    fclose(fp);
}


std::vector<std::string>& ArgvFile::launchArgs()
{
    return _launchArgs;
}

const std::string& ArgvFile::launchPath()
{
    return _launchPath;
}