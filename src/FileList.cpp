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
#include "Helper.h"


FileList::FileList() {
}

FileList::~FileList() {
  Logger::debug("~FileList %s", m_filename.c_str());
  m_entries.clear();
}

bool FileList::load(const std::string& filename) {
  m_filename = filename;

  Logger::debug("loading file %s", m_filename.c_str());

  std::ifstream in(m_filename);
  if (in.fail()) {
    Logger::warn("loading file %s failed", m_filename.c_str());
    return false;
  }

  std::stringstream buffer;
  buffer << in.rdbuf();
  std::string str = buffer.str();

  m_entries.clear();
  struct json_object* root = json_tokener_parse(str.c_str());

  if (!Helper::getObject(root, "name", true, m_filename, &m_name)) {
    return false;
  }

  struct json_object* entries;
  if (json_object_object_get_ex(root, "entries", &entries)) {
    for (int i = 0; i < json_object_array_length(entries); i++) {
      struct json_object* entry = json_object_array_get_idx(entries, i);
      
      struct FileListEntry add;
      if (!Helper::getObject(entry, "number", true, m_filename, &add.number)) {
        continue;
      }
      if (!Helper::getObject(entry, "name", true, m_filename, &add.name)) {
        continue;
      }
      m_entries.push_back(add);
    }
  } else {
      Logger::debug("no entries section found in json file %s", m_filename.c_str());
  }
  json_object_put(root); // free
  return true;
}

std::string FileList::getName() {
  return m_name;
}

bool FileList::isListed(const std::string& number, std::string* pName) {
  for(size_t i = 0; i < m_entries.size(); i++) {
    struct FileListEntry* entry = &m_entries[i];
    const char* s = entry->number.c_str();
    if (strncmp(s, number.c_str(), strlen(s)) == 0) {
      Logger::debug("FileList::isListed(number='%s') matched with '%s'/'%s' in file %s",
        number.c_str(), s, entry->name.c_str(), m_filename.c_str());
      *pName = entry->name;
      return true;
    }
  }
  return false;
}

void FileList::dump() {
  printf("Name=%s:\n", m_name.c_str());
  for(size_t i = 0; i < m_entries.size(); i++) {
    struct FileListEntry* entry = &m_entries[i];
    printf("'%s'/'%s'\n", entry->number.c_str(), entry->name.c_str());
  }
}

