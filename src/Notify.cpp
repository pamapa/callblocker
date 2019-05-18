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

#include "Notify.h" // API

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <poll.h>

#include "Logger.h"


#define EVENT_SIZE      (sizeof(struct inotify_event))
#define EVENT_BUF_LEN   (1024 * ( EVENT_SIZE + 16 ))


Notify::Notify(const std::string& rPathname, uint32_t mask) {
  Logger::debug("Notify::Notify(%s, %d)", rPathname.c_str(), mask);

  m_FD = inotify_init();
  if (m_FD < 0) {
    Logger::warn("inotify_init failed (%s)", strerror(errno));
    return;
  }

  m_WD = inotify_add_watch(m_FD, rPathname.c_str(), mask);
  if (m_WD < 0) {
    Logger::warn("inotify_add_watch failed (%s)", strerror(errno));
    return;
  }
}

Notify::~Notify() {
  Logger::debug("Notify::~Notify()");

  if (m_WD > 0) {
    inotify_rm_watch(m_FD, m_WD);
  }
  if (m_FD > 0) {
    close(m_FD);
  }
  m_FD = m_WD = -1;
}

bool Notify::hasChanged() {
  if (m_WD < 0) {
    return false;
  }

  bool res = false;
  struct pollfd pfd = {m_FD, POLLIN | POLLPRI, 0};
  int ret = poll(&pfd, 1, 0);
  if (ret < 0) {
    // Logger::warn("poll failed with %s", strerror(errno));
    // also happens when application is shutdown...
  } else if (ret == 0) {
    // no event to read
  } else {
    // event to read
    char buffer[EVENT_BUF_LEN];
    int length = read(m_FD, buffer, sizeof(buffer));
    int i = 0;
    while (i < length) {
      struct inotify_event *event = (struct inotify_event*)&buffer[i];
      //Logger::debug("The file %s was touched", event->name);
      if (event->len) {
        res = true;
        break;
      }
      i += EVENT_SIZE + event->len;
    }
  }
  return res;
}

