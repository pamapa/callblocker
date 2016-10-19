/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2016 Patrick Ammann <pammann@gmx.net>

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

#include "FileListsNotified.h" // API

#include <string>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>

#include "Logger.h"
#include "Utils.h"


FileListsNotified::FileListsNotified(const std::string& rPathname) : Notify(rPathname, IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO) {
  Logger::debug("FileListsNotified::FileListsNotified()...");
  m_pathname = rPathname;

  if (pthread_mutex_init(&m_mutexLock, NULL) != 0) {
    Logger::warn("pthread_mutex_init failed");
  }

  load();
}

FileListsNotified::~FileListsNotified() {
  Logger::debug("FileListsNotified::~FileListsNotified()...");
  clear();
}

void FileListsNotified::run() {
  if (hasChanged()) {
    Logger::info("reload %s", m_pathname.c_str());

    pthread_mutex_lock(&m_mutexLock);
    clear();
    load();
    pthread_mutex_unlock(&m_mutexLock);
  }
}

bool FileListsNotified::getEntry(const std::string& rNumber, std::string* pListName, std::string* pCallerName) {
  bool ret = false;
  pthread_mutex_lock(&m_mutexLock);
  for(size_t i = 0; i < m_lists.size(); i++) {
    if (m_lists[i]->getEntry(rNumber, pCallerName)) {
      *pListName = m_lists[i]->getName();
      ret = true;
      break;
    }
  }
  pthread_mutex_unlock(&m_mutexLock);
  return ret;
}

void FileListsNotified::load() {
  DIR* dir = opendir(m_pathname.c_str());
  if (dir == NULL) {
    Logger::warn("open directory %s failed", m_pathname.c_str());
    return;
  }

  Logger::debug("loading directory %s", m_pathname.c_str());
  struct dirent* entry = readdir(dir);
  while (entry != NULL) {
    if ((entry->d_type & DT_DIR) == 0) {
      size_t len = strlen(entry->d_name);
      if (len >= 5 && strcmp(entry->d_name + len - 5, ".json") == 0) {
        // only reading .json files
        std::string filename = Utils::pathJoin(m_pathname, entry->d_name);
        FileList* l = new FileList(filename);
        if (l->load()) {
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

void FileListsNotified::clear() {
  for(size_t i = 0; i < m_lists.size(); i++) {
    delete m_lists[i];
  }
  m_lists.clear();
}

void FileListsNotified::dump() {
  pthread_mutex_lock(&m_mutexLock);
  for(size_t i = 0; i < m_lists.size(); i++) {
    FileList* l = m_lists[i];
    l->dump();
  }
  pthread_mutex_unlock(&m_mutexLock);
}

