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

#include <iostream>

#include "easysave/ini.hpp"

using namespace easysave;

void ini::SetString(std::string section, std::string key_name, std::string key_data) {
  int section_index = m_match_section_index(section);
  if (section_index < 0) {
    // Section does not exist; add it
    m_sections.push_back(section);
    section_index = m_sections.size() - 1;
  }

  int key_index = m_match_key_index(section_index, key_name);
  if (key_index < 0) {
    // Key does not exist; we create it from scratch
    std::cout << "Creating key in section " << m_sections[section_index]
              << std::endl;
    m_keys.push_back((m_ini_key_t){section_index, key_name, key_data});
    return;
  }

  m_keys[key_index].data = key_data;
}

void ini::SetInt(std::string section, std::string key_name, int key_data) {
  return SetString(section, key_name, std::to_string(key_data));
}