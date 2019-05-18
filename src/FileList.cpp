/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2019 Patrick Ammann <pammann@gmx.net>

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
#include <algorithm>
#include <stdio.h>
#include <string.h>
#include <json-c/json.h>

#include "Logger.h"
#include "Utils.h"


FileList::FileList(const std::string& filename, const std::string& name) {
  Logger::debug("FileList::FileList(filename='%s', name='%s')", filename.c_str(), name.c_str());
  m_filename = filename;
  m_name = name;
}

FileList::FileList(const std::string& filename) : FileList(filename, Utils::pathBasename(filename)) {
}

FileList::~FileList() {
  Logger::debug("FileList::~FileList() of '%s'", m_filename.c_str());
  m_entries.clear();
}

bool FileList::load() {
  Logger::debug("FileList::load() of '%s'", m_filename.c_str());
  
  m_entries.clear();

  struct json_object* root;
  if (!Utils::loadJson(m_filename, &root)) {
    return false;
  }

  if (!Utils::getObject(root, "name", true, m_filename, &m_name, "")) {
    return false;
  }

  struct json_object* entries;
  if (json_object_object_get_ex(root, "entries", &entries)) {
    for (int i = 0; i < json_object_array_length(entries); i++) {
      struct json_object* entry = json_object_array_get_idx(entries, i);
      
      struct FileListEntry add;
      if (!Utils::getObject(entry, "number", true, m_filename, &add.number, "")) {
        continue;
      }
      if (!Utils::getObject(entry, "name", true, m_filename, &add.name, "")) {
        continue;
      }
      (void)Utils::getObject(entry, "date_created", false, m_filename, &add.date_created, std::chrono::system_clock::now());

      m_entries.push_back(add);
    }
  } else {
      Logger::debug("no entries section found in json file %s", m_filename.c_str());
  }
  json_object_put(root); // free
  return true;
}

bool FileList::save() {
  Logger::debug("FileList::save() of '%s'", m_filename.c_str());
  
  std::ofstream out(m_filename);
  if (out.fail()) {
    Logger::warn("saving file %s failed", m_filename.c_str());
    return false;
  }
  
  out << "{\n";
  out << "  \"name\": \"" << m_name << "\",\n";
  out << "  \"entries\": [\n"; 
  for(size_t i = 0; i < m_entries.size(); i++) {
    struct FileListEntry* entry = &m_entries[i];
    out << "      {\n";
    out << "        \"number\": \"" << entry->number << "\",\n";
    out << "        \"name\": \"" << entry->name << "\",\n";
    out << "        \"date_created\": \"" << Utils::formatTime(entry->date_created) << "\"\n";
    out << "      }";
    if (i + 1 < m_entries.size()) {
      out << ",\n";
    } else {
      out << "\n";
    }
  }
  out << "  ]\n";
  out << "}\n";
  
  out.close();
  return true;
}

std::string FileList::getName() {
  return m_name;
}

bool FileList::getEntry(const std::string& rNumber, std::string* pName) {
  for(size_t i = 0; i < m_entries.size(); i++) {
    struct FileListEntry* entry = &m_entries[i];
    const char* s = entry->number.c_str();
    if (strncmp(s, rNumber.c_str(), strlen(s)) == 0) {
      Logger::debug("FileList::isListed(number='%s') matched with '%s'/'%s' in file %s",
        rNumber.c_str(), s, entry->name.c_str(), m_filename.c_str());
      if (pName != NULL) *pName = entry->name;
      return true;
    }
  }
  return false;
}

void FileList::addEntry(const std::string& rNumber, const std::string& rCallerName) {
  Logger::debug("FileList::addEntry(rNumber='%s', rCallerName='%s') to '%s'",
    rNumber.c_str(), rCallerName.c_str(), m_filename.c_str());

  if (getEntry(rNumber, NULL)) {
    Logger::warn("internal error, entry '%s' already part of list", rNumber.c_str());
    return;
  }

  struct FileListEntry add;
  add.number = rNumber;
  add.name = rCallerName;
  add.date_created = std::chrono::system_clock::now();

  m_entries.push_back(add);
}

void FileList::removeEntry(const std::string& rNumber) {
  m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
                  [rNumber](FileListEntry e) {
                    const char* s = e.number.c_str();
                    if (strncmp(s, rNumber.c_str(), strlen(s)) == 0) {
                      return true;
                    }
                    return false;
                  }),
                  m_entries.end());
}

bool FileList::eraseAged(size_t maxDays) {
  Logger::debug("FileList::eraseAged(maxDays=%zu) in '%s'", maxDays, m_filename.c_str());
  bool changed = false;

  std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
  m_entries.erase(std::remove_if(m_entries.begin(), m_entries.end(),
                  [maxDays, &changed, now](FileListEntry e) {
                    size_t hours = std::chrono::duration_cast<std::chrono::hours>(now.time_since_epoch() - e.date_created.time_since_epoch()).count();
                    if (hours > maxDays * 24) {
                      changed = true;
                      return true;
                    }
                    return false;
                  }),
                  m_entries.end());

  return changed;
}

void FileList::dump() {
  printf("Name='%s': [\n", m_name.c_str());
  for(size_t i = 0; i < m_entries.size(); i++) {
    struct FileListEntry* entry = &m_entries[i];
    printf("  '%s', '%s', %s\n", entry->name.c_str(), entry->number.c_str(), Utils::formatTime(entry->date_created).c_str());
  }
  printf("]\n");
}

