#include <string>
#include <cstdio>
#include <vector>

#ifndef __DS_ARGV__
#define __DS_ARGV__

class ArgvFile
{
    public:
        ArgvFile(const std::string& filename);
        ~ArgvFile() {}

    public:
        const std::string& launchPath();
        std::vector<std::string>& launchArgs();

    private:
        std::string _launchPath;
        std::vector<std::string> _launchArgs;

};

#endif