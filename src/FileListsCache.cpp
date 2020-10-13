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
  Logger::debug("FileListsCache::FileListsCache(rPathname='%s')", rPathname.c_str());
  m_pathname = rPathname;

  if (pthread_mutex_init(&m_mutexLock, nullptr) != 0) {
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
  Logger::debug("FileListsCache::~FileListsCache() of '%s'", m_pathname.c_str());
  
  for (const auto& l : m_lists) {
    delete(l.list);
  }
}

void FileListsCache::run() {
  bool doEraseAged = false;
  std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
  if (m_nextEraseAgedTime < now) {
    m_nextEraseAgedTime = now + std::chrono::hours(NEXT_ERASE_AGED_TIME_HOURS);
    doEraseAged = true;
  }

  for (auto& l : m_lists) {
    if (hasChanged()) {
      Logger::info("reload %s %s", l.list->getName().c_str(), m_pathname.c_str());
      load();
    }

    if (doEraseAged) {
      pthread_mutex_lock(&m_mutexLock);
      bool changed = l.list->eraseAged(MAX_AGE_IN_DAYS);
      l.saveNeeded = changed;
      pthread_mutex_unlock(&m_mutexLock);
    }

    if (l.saveNeeded) {
      Logger::info("save %s %s", l.list->getName().c_str(), m_pathname.c_str());

      pthread_mutex_lock(&m_mutexLock);
      l.list->save();
      (void)hasChanged(); // avoid useless reload, because of above save
      l.saveNeeded = false;
      pthread_mutex_unlock(&m_mutexLock);
    }
  }
}

bool FileListsCache::getEntryByNumber(const CacheType type, const std::string& rNumber, std::string* pCallerName) {
  bool ret;
  pthread_mutex_lock(&m_mutexLock);
  auto& l = m_lists[(size_t)type];
  ret = l.list->getEntryByNumber(rNumber, pCallerName);
  pthread_mutex_unlock(&m_mutexLock);
  return ret;
}

void FileListsCache::addEntry(const CacheType type, const std::string& rNumber, const std::string& rCallerName) {
  pthread_mutex_lock(&m_mutexLock);
  auto& l = m_lists[(size_t)type];
  l.list->addEntry(rNumber, rCallerName);
  l.saveNeeded = true;
  pthread_mutex_unlock(&m_mutexLock);
}

void FileListsCache::load() {
  Logger::debug("FileListsCache::load() of '%s'", m_pathname.c_str());

  pthread_mutex_lock(&m_mutexLock);
  for (auto& l : m_lists) {
    l.list->load();
    l.saveNeeded = false;
  }
  pthread_mutex_unlock(&m_mutexLock);
}

void FileListsCache::dump() {
  pthread_mutex_lock(&m_mutexLock);
  for (const auto& l : m_lists) {
    l.list->dump();
  }
  pthread_mutex_unlock(&m_mutexLock);
}
