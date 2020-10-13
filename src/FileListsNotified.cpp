/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2020 Patrick Ammann <pammann@gmx.net>

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
  Logger::debug("FileListsNotified::FileListsNotified(rPathname='%s')", rPathname.c_str());
  m_pathname = rPathname;

  if (pthread_mutex_init(&m_mutexLock, nullptr) != 0) {
    Logger::warn("pthread_mutex_init failed");
  }

  load();
}

FileListsNotified::~FileListsNotified() {
  Logger::debug("FileListsNotified::~FileListsNotified() of '%s'", m_pathname.c_str());
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

bool FileListsNotified::getEntryByNumber(const std::string& rNumber, std::string* pListName, std::string* pCallerName) {
  bool ret = false;
  pthread_mutex_lock(&m_mutexLock);
  for (const auto& list : m_lists) {
    if (list->getEntryByNumber(rNumber, pCallerName)) {
      *pListName = list->getName();
      ret = true;
      break;
    }
  }
  pthread_mutex_unlock(&m_mutexLock);
  return ret;
}

bool FileListsNotified::getEntryByName(const std::string& rCallerName, std::string* pListName) {
  bool ret = false;
  pthread_mutex_lock(&m_mutexLock);
  for (const auto& list : m_lists) {
    if (list->getEntryByName(rCallerName)) {
      *pListName = list->getName();
      ret = true;
      break;
    }
  }
  pthread_mutex_unlock(&m_mutexLock);
  return ret;
}

void FileListsNotified::load() {
  Logger::debug("FileListsNotified::load() of '%s'", m_pathname.c_str());
  DIR* dir = opendir(m_pathname.c_str());
  if (dir == nullptr) {
    Logger::warn("open directory %s failed", m_pathname.c_str());
    return;
  }

  struct dirent* entry = readdir(dir);
  while (entry != nullptr) {
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
  for (const auto& list : m_lists) {
    delete list;
  }
  m_lists.clear();
}

void FileListsNotified::dump() {
  pthread_mutex_lock(&m_mutexLock);
  for (const auto& list : m_lists) {
    list->dump();
  }
  pthread_mutex_unlock(&m_mutexLock);
}

