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

#include "Lists.h" // API

#include <string>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>

#include "Logger.h"


Lists::Lists(std::string dirname) : Notify(dirname, IN_CLOSE_WRITE | IN_DELETE) {
  m_dirname = dirname;
  load();
}

Lists::~Lists() {
  clear();
}

void Lists::watch() {
  if (changed()) {
    Logger::info("reload %s", m_dirname.c_str());

    // TODO: mutex....
    clear();
    load();
  }
}

// TODO: mutex....
bool Lists::isListed(const char* number) {
  bool ret = false;
  for(size_t i = 0; i < m_lists.size(); i++) {
    if (m_lists[i]->hasNumber(number)) {
      ret = true;
      break;
    }
  }
  return ret;
}

void Lists::load() {
  DIR* dir = opendir(m_dirname.c_str());
  if (dir == NULL) {
    Logger::warn("open directory %s failed", m_dirname.c_str());
    return;
  }

  Logger::debug("loading directory %s", m_dirname.c_str());
  struct dirent* entry = readdir(dir);
  while (entry != NULL) {
    if ((entry->d_type & DT_DIR) == 0) {
      std::string filename = m_dirname + "/";
      filename += entry->d_name;
      List* l = new List();
      if (l->load(filename)) {
        m_lists.push_back(l);
      } else {
        delete l;
      }        
    }
    entry = readdir(dir);
  }
  closedir(dir);
}

void Lists::clear() {
  for(size_t i = 0; i < m_lists.size(); i++) {
    delete m_lists[i];
  }
  m_lists.clear();
}

void Lists::dump() {
  for(size_t i = 0; i < m_lists.size(); i++) {
    List* l = m_lists[i];
    printf("Filename=%s\n", l->getFilename());
    l->dump();
  }
}

