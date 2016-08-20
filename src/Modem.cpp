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

#include "Modem.h" // API

#include <string>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#include "Logger.h"
#include "Utils.h"


#define CHARWAIT_TIME_DSEC    1             // deciseconds (1dsec = 0.1sec)
#define READWAIT_TIME_USEC    (150 * 1000)  // 150 miliseconds


Modem::Modem() {
  Logger::debug("Modem::Modem()...");
  m_FD = -1;
}

Modem::~Modem() {
  Logger::debug("Modem::~Modem()...");

  if (m_FD != -1) {
    (void)sendCommand("ATZ"); // reset setting to defaults
    (void)tcsetattr(m_FD, TCSANOW, &m_origTermios);
    close(m_FD);
    m_FD = -1;
  }
}

bool Modem::open(std::string name) {
  Logger::debug("Modem::open(%s)...", name.c_str());

  m_name = name;
  m_FD = ::open(name.c_str(), O_RDWR|O_NOCTTY);
  if (m_FD < 0) {
    Logger::warn("[%s] failed to open (%s)", name.c_str(), strerror(errno));
    m_FD = -1;
    return false;
  }

  // get original options
  if (tcgetattr(m_FD, &m_origTermios) < 0) {
    Logger::warn("[%s] tcgetattr failed (%s)", name.c_str(), strerror(errno));
    return false;
  }

#if 0
  int flags = fcntl(m_FD, F_GETFL, 0);
  if (flags < 0) {
    Logger::warn("fcntl F_GETFL failed on device %s (%s)", m_settings.device.c_str(), strerror(errno));
    return false;
  }
  flags = fcntl(m_FD, F_SETFL, flags | O_NONBLOCK);
  if (flags < 0) {
    Logger::warn("fcntl F_SETFL failed on device %s (%s)", m_settings.device.c_str(), strerror(errno));
    return false;
  }
#endif

  struct termios options = m_origTermios;

  // No parity (8N1)
  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;

  // hardware flow control
  options.c_cflag |= CRTSCTS;

  // ignore modem control lines and enable receiver
  options.c_cflag |= (CLOCAL | CREAD);

  // raw input
  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

  // raw out put
  options.c_oflag &= ~OPOST;

  // A read returns immediately with up to the number of bytes
  // requested. It returns the number read; zero if none available
  options.c_cc[VMIN]  = 0;
  options.c_cc[VTIME] = CHARWAIT_TIME_DSEC;

  // baud rate (caller ID is sent at 1200 baud)
  cfsetispeed(&options, B1200);
  cfsetospeed(&options, B1200);

  // apply options
  if (tcsetattr(m_FD, TCSANOW, &options) < 0) {
    Logger::warn("[%s] tcgetattr failed (%s)", m_name.c_str(), strerror(errno));
    return false;
  }

  return true;
}

bool Modem::sendCommand(std::string cmd) {
  Logger::debug("[%s] send %s command...", m_name.c_str(), cmd.c_str());
  std::string sendCmd = cmd + "\r\n"; // CRLF

  int len = write(m_FD, sendCmd.c_str(), strlen(sendCmd.c_str()));
  if (len != (int)strlen(sendCmd.c_str())) {
    Logger::warn("[%s] write command '%s' failed (%s)", m_name.c_str(), cmd.c_str(), strerror(errno));
    return false;
  }

  // read until OK or ERROR response detected or number of tries exceeded
  char buffer[256];
  int tries, size;
  bool ret = false;
  for (size = tries = 0; tries < 10; tries++) {
    (void)usleep(READWAIT_TIME_USEC);

    int num = read(m_FD, buffer + size, sizeof(buffer) - size - 1);
    if (num < 0) continue;

    size += num;
    if (size != 0) {
      // check response
      buffer[size] = '\0';
      if (strstr(buffer, "OK")) {
        ret = true;
        break;
      }
      if (strstr(buffer, "ERROR")) {
        Logger::warn("[%s] received 'ERROR' for command '%s'", m_name.c_str(), cmd.c_str());
        ret = false;
        break;
      }
    }
  }
  buffer[size] = '\0';
  
  std::string str = buffer;
  Utils::trim(&str);
  Logger::debug("[%s] received '%s' tries=%d ret=%d", m_name.c_str(), str.c_str(), tries, ret);
  return ret;
}

bool Modem::getData(std::string* data) {
  bool res = false;
  struct pollfd pfd = {m_FD, POLLIN | POLLPRI, 0};
  int ret = poll(&pfd, 1, 0);
  if (ret < 0) {
    // Logger::warn("poll failed with %s", strerror(errno));
  } else if (ret == 0) {
    // no data to read
  } else {
    // data to read
    char buffer[256];
    int num = read(m_FD, buffer, sizeof(buffer));
    if (num > 0) {
      buffer[num] = '\0';
      std::string str = buffer;
      Utils::trim(&str);
      *data = str;
      Logger::debug("[%s] received '%s'", m_name.c_str(), data->c_str());
      res = true;
    }
  }
  return res;
}

