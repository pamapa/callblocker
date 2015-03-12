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

#include "FileLists.h" // API

#include <string>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>

#include "Logger.h"


FileLists::FileLists(const std::string& dirname) : Notify(dirname, IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO) {
  m_dirname = dirname;
  load();
}

FileLists::~FileLists() {
  Logger::debug("~FileLists");
  clear();
}

void FileLists::run() {
  if (hasChanged()) {
    Logger::info("reload %s", m_dirname.c_str());
    // TODO: mutex....
    clear();
    load();
  }
}

// TODO: mutex....
bool FileLists::isListed(const std::string& number, std::string* pMsg) {
  bool ret = false;
  for(size_t i = 0; i < m_lists.size(); i++) {
    if (m_lists[i]->hasNumber(number)) {
      *pMsg = m_lists[i]->getBaseFilename();
      ret = true;
      break;
    }
  }
  return ret;
}

void FileLists::load() {
  DIR* dir = opendir(m_dirname.c_str());
  if (dir == NULL) {
    Logger::warn("open directory %s failed", m_dirname.c_str());
    return;
  }

  Logger::debug("loading directory %s", m_dirname.c_str());
  struct dirent* entry = readdir(dir);
  while (entry != NULL) {
    if ((entry->d_type & DT_DIR) == 0) {
      size_t len = strlen(entry->d_name);
      if (len >= 5 && strcmp(entry->d_name + len - 5, ".json") == 0) {
        // only reading .json files      
        std::string filename = m_dirname + "/";
        filename += entry->d_name;
        FileList* l = new FileList();
        if (l->load(filename)) {
          m_lists.push_back(l);
        } else {
          delete l;
        }
      }
    }
    entry = readdir(dir);
  }
  closedir(dir);
}

void FileLists::clear() {
  for(size_t i = 0; i < m_lists.size(); i++) {
    delete m_lists[i];
  }
  m_lists.clear();
}

void FileLists::dump() {
  for(size_t i = 0; i < m_lists.size(); i++) {
    FileList* l = m_lists[i];
    printf("Filename=%s\n", l->getFilename());
    l->dump();
  }
}

