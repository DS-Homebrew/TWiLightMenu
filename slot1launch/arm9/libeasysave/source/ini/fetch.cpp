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

#include "easysave/ini.hpp"

using namespace easysave;

std::string ini::GetString(std::string section, std::string key_name, std::string default_value)
{
  std::string temp = GetString(section, key_name);
  if (temp == "") {
	ini::SetString(section, key_name, default_value);
	temp = default_value;
  }

  return temp;
}
std::string ini::GetString(std::string section, std::string key_name) {
  int section_index = m_match_section_index(section);
  if (section_index < 0)
    return "";

  int key_index = m_match_key_index(section_index, key_name);
  if (key_index < 0)
    return "";

  // Remove leading and trailing quotes
  if (m_keys[key_index].data[0] == '\"' &&
      m_keys[key_index].data[m_keys[key_index].data.size() - 1] == '\"')
    return m_keys[key_index].data.substr(1, m_keys[key_index].data.size() - 2);
  return m_keys[key_index].data;
}

int ini::GetInt(std::string section, std::string key_name) {
	return strtol(GetString(section, key_name).c_str(), NULL, 0);
}

int ini::GetInt(std::string section, std::string key_name, int default_value) {
	return strtol(GetString(section, key_name, std::to_string(default_value)).c_str(), NULL, 0);
}