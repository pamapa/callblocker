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

#include "FileListsCache.h" // API

#include <string>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>

#include "Logger.h"
#include "Utils.h"


#define MAX_AGE_IN_DAYS             365
#define NEXT_ERASE_AGED_TIME_HOURS  8


FileListsCache::FileListsCache(const std::string& rPathname) : Notify(rPathname, IN_CLOSE_WRITE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO) {
  Logger::debug("FileListsCache::FileListsCache()...");
  m_pathname = rPathname;

  if (pthread_mutex_init(&m_mutexLock, NULL) != 0) {
    Logger::warn("pthread_mutex_init failed");
  }
  
  static const char* names[] = {"onlinelookup", "onlinecheck"};
  for (size_t i = 0; i < sizeof(m_lists)/sizeof(m_lists[0]); i++) {
    std::string filename = Utils::pathJoin(m_pathname, names[i]) + ".json";
    m_lists[i].list = new FileList(filename, names[i]);
    m_lists[i].saveNeeded = false;

    // avoid useless warnings: create empty if files not exists
    if (!Utils::pathExists(filename)) {
      m_lists[i].list->save();
    }
  }
  (void)hasChanged(); // avoid useless reload, because of above save

  load();

  m_nextEraseAgedTime = std::chrono::steady_clock::time_point::min();
}

FileListsCache::~FileListsCache() {
  Logger::debug("FileListsCache::~FileListsCache()...");
  
  for (size_t i = 0; i < sizeof(m_lists)/sizeof(m_lists[0]); i++) {
    delete(m_lists[i].list);
  }
}

void FileListsCache::run() {
  bool doEraseAged = false;
  std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
  if (m_nextEraseAgedTime < now) {
    m_nextEraseAgedTime = now + std::chrono::hours(NEXT_ERASE_AGED_TIME_HOURS);
    doEraseAged = true;
  }

  for (size_t i = 0; i < sizeof(m_lists)/sizeof(m_lists[0]); i++) {
    if (hasChanged()) {
      Logger::info("reload %lu %s", i, m_pathname.c_str());
      load();
    }

    if (doEraseAged)
    {
      pthread_mutex_lock(&m_mutexLock);
      bool changed = m_lists[i].list->eraseAged(MAX_AGE_IN_DAYS);
      m_lists[i].saveNeeded = changed;
      pthread_mutex_unlock(&m_mutexLock);
    }

    if (m_lists[i].saveNeeded) {
      Logger::info("save %lu %s", i, m_pathname.c_str());

      pthread_mutex_lock(&m_mutexLock);
      m_lists[i].list->save();
      (void)hasChanged(); // avoid useless reload, because of above save
      m_lists[i].saveNeeded = false;
      pthread_mutex_unlock(&m_mutexLock);
    }
  }
}

void FileListsCache::addEntry(const CacheType type, const std::string& rNumber, const std::string& rCallerName) {
  pthread_mutex_lock(&m_mutexLock);
  m_lists[(size_t)type].list->addEntry(rNumber, rCallerName);
  m_lists[(size_t)type].saveNeeded = true;
  pthread_mutex_unlock(&m_mutexLock);
}

bool FileListsCache::getEntry(const CacheType type, const std::string& rNumber, std::string* pCallerName) {
  bool ret;
  pthread_mutex_lock(&m_mutexLock);
  ret = m_lists[(size_t)type].list->getEntry(rNumber, pCallerName);
  pthread_mutex_unlock(&m_mutexLock);
  return ret;
}

void FileListsCache::load() {
  Logger::debug("loading directory %s", m_pathname.c_str());

  pthread_mutex_lock(&m_mutexLock);
  for (size_t i = 0; i < sizeof(m_lists)/sizeof(m_lists[0]); i++) {
    m_lists[i].list->load();
    m_lists[i].saveNeeded = false;
  }
  pthread_mutex_unlock(&m_mutexLock);
}

void FileListsCache::dump() {
  pthread_mutex_lock(&m_mutexLock);
  for (size_t i = 0; i < sizeof(m_lists)/sizeof(m_lists[0]); i++) {
    FileList* l = m_lists[i].list;
    l->dump();
  }
  pthread_mutex_unlock(&m_mutexLock);
}

