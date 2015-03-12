/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2015 Patrick Ammann <pammann@gmx.net>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "FileList.h" // API

#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <json-c/json.h>

#include "Logger.h"


FileList::FileList() {
}

FileList::~FileList() {
  Logger::debug("~FileList %s", getFilename());
  m_entries.clear();
}

bool FileList::load(const std::string& filename) {
  m_filename = filename;

  Logger::debug("loading file %s", getFilename());

  std::ifstream in(getFilename());
  if (in.fail()) {
    Logger::warn("loading file %s failed", getFilename());
    return false;
  }

  std::stringstream buffer;
  buffer << in.rdbuf();
  std::string str = buffer.str();

  m_entries.clear();
  struct json_object* root = json_tokener_parse(str.c_str());

  struct json_object* entries;
  if (json_object_object_get_ex(root, "entries", &entries)) {
    for (size_t i = 0; i < json_object_array_length(entries); i++) {
      struct json_object* entry = json_object_array_get_idx(entries, i);
      struct json_object* n;
      if (!json_object_object_get_ex(entry, "number", &n)) {
        Logger::warn("invalid json file %s", getFilename());
        continue;
      }
      if (json_object_get_type(n) != json_type_string) {
        Logger::warn("invalid json file %s", getFilename());
        continue;
      }
      const char* s = json_object_get_string(n);
      std::string str = s;
      m_entries.push_back(str);
    }
  } else {
      Logger::debug("no entries section found in json file %s", getFilename());
  }
  json_object_put(root); // free
  return true;
}

const char* FileList::getFilename() {
  return m_filename.c_str();
}

bool FileList::hasNumber(const std::string& number) {
  for(size_t i = 0; i < m_entries.size(); i++) {
    const char* s = m_entries[i].c_str();
    if (strncmp(s, number.c_str(), strlen(s)) == 0) {
      Logger::debug("FileList::hasNumber: number '%s' matched with '%s' in file %s", number.c_str(), s, getFilename());
      return true;
    }
  }
  return false;
}

void FileList::dump() {
  for(size_t i = 0; i < m_entries.size(); i++) {
    printf("%s\n", m_entries[i].c_str());
  }
}

