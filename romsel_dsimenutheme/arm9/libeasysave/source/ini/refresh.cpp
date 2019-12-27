// libeasysave

/*
MIT License

Copyright (c) 2019 Jonathan Archer

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <fstream>
#include <regex>

#include "easysave/ini.hpp"

using namespace easysave;

size_t ini::refresh() {
  std::ifstream file;

  file.open(m_filename);

  if (!file.is_open())
    return 1;

  // Clear out our variables
  m_sections.clear();
  m_keys.clear();

  // Default section
  m_sections.push_back("default");
  int index = m_sections.size() - 1;

  // Loop through every line
  while (!file.eof()) {
    std::string line;
    std::getline(file, line);

    // Ensure there is no carriage return
    line = std::regex_replace(line, std::regex("\r+$"), "");

    // Ignore if comment
    if (line[0] == ';')
      continue;

    // Check if section
    if (line[0] == '[' && line.find(']') != std::string::npos) {
      // Append if not a duplicate
      bool is_duplicate = false;
      for (size_t i = 0; i < m_sections.size(); i++)
        if (m_sections[i] == line.substr(1, line.find(']') - 1)) {
          is_duplicate = true;
          index = i;
        }
      // Otherwise add
      if (!is_duplicate) {
        m_sections.push_back(line.substr(1, line.find(']') - 1));
        index = m_sections.size() - 1;
      }
    }

    // Check if valid key, then append if it is
    if (line.find('=') != std::string::npos) {
      m_keys.push_back(
          (m_ini_key_t){index, line.substr(0, line.find('=')),
                        line.substr(line.find('=') + 1, line.size())});

      // Remove leading and trailing whitespace
      m_keys.back().name =
          std::regex_replace(m_keys.back().name, std::regex("^ +| +$"), "");
      m_keys.back().data =
          std::regex_replace(m_keys.back().data, std::regex("^ +| +$"), "");
    }
  }

  file.close();
  return 0;
}