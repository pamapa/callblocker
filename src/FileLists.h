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

#ifndef FILELISTS_H
#define FILELISTS_H

#include <vector>
#include <pthread.h>

#include "FileList.h"
#include "Notify.h"


class FileLists : public Notify {
private:
  pthread_mutex_t m_mutexLock;
  std::string m_pathname;
  std::vector<FileList*> m_lists;

public:
  FileLists(const std::string& rDirname);
  virtual ~FileLists();
  void run();

  bool isListed(const std::string& rNumber, std::string* pListName, std::string* pCallerName);

  void dump();

private:
  void load();
  void clear();
};

#endif

