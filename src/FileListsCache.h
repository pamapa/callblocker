/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2017 Patrick Ammann <pammann@gmx.net>

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

#ifndef FILELISTS_CACHE_H
#define FILELISTS_CACHE_H

#include <pthread.h>
#include <chrono>

#include "FileList.h"
#include "Notify.h"


enum class CacheType {
  OnlineLookup = 0,
  OnlineCheck
};

struct CacheFileList {
  FileList* list;
  bool saveNeeded;
};

class FileListsCache : public Notify {
private:
  pthread_mutex_t m_mutexLock;
  std::string m_pathname;
  
  CacheFileList m_lists[2];

  std::chrono::time_point<std::chrono::steady_clock> m_nextEraseAgedTime;

public:
  FileListsCache(const std::string& rPathname);
  virtual ~FileListsCache();
  void run();
  
  bool getEntry(const CacheType type, const std::string& rNumber, std::string* pCallerName);
  void addEntry(const CacheType type, const std::string& rNumber, const std::string& rCallerName);

  void dump();

private:
  void load();
};

#endif
