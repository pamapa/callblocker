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

#include "AnalogPhone.h" // API

#include <string>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>

#include "Logger.h"

/*
  AT commands:
  http://support.usr.com/support/5637/5637-ug/ref_data.html
*/
#define AT_Z_STR        "AT Z S0=0 E0 Q0 V1"
#define AT_CID_STR      "AT+VCID=1"
#define AT_HANGUP_STR   "ATH0"
#define AT_PICKUP_STR   "ATH1"

#define CHARWAIT_TIME_DSEC    1       /* deciseconds (1dsec = 0.1sec) */
#define READWAIT_TIME_USEC    100000  /* microseconds */


/*
 // Tell the modem to return caller ID. Note: different

ATZ
OK
AT+VCID=1
OK

RING

DATE=0306
TIME=1517
NMBR=**610
NAME=aasdasdd

RING


*/

static time_t getMonotonicTime(void) {
  struct timespec tp;
  (void)clock_gettime(CLOCK_MONOTONIC, &tp);
  return tp.tv_sec;
}


AnalogPhone::AnalogPhone(FileLists* whitelists, FileLists* blacklists) : Phone(whitelists, blacklists) {
  m_FD = -1;
  m_numRings = m_ringTime = 0;
  m_foundCID = false;
}

AnalogPhone::~AnalogPhone() {
  Logger::debug("~AnalogPhone...");
  if (m_FD != -1) {
    (void)tcsetattr(m_FD, TCSANOW, &m_origTermios);
    close(m_FD);
    m_FD = -1;
  }
}

bool AnalogPhone::init(struct SettingAnalogPhone* phone) {
  Logger::debug("AnalogPhone::init(%s)...", phone->toString().c_str());
  m_settings = *phone; // struct copy

  if (!openDevice()) {
    return false;
  }

  if (!sendCommand(AT_Z_STR)) {
    return false;
  }
  if (!sendCommand(AT_CID_STR)) {
    return false;
  }


  return true;
}

// load this into a seperate thread, needed for LiveAPI access, which may take some time...,
// or offload LiveAPI access itself into a seperate thread? YES?
void AnalogPhone::run() {
  std::string data;
  if (getData(&data)) {
    if (data == "RING") {
      m_numRings++;
      m_ringTime = getMonotonicTime();

      // handle no caller ID
      if (m_numRings == 2) {
        if (!m_foundCID) {
          // TODO: no CID received -> notice
        }
      }
    } else {
      std::vector<std::string> lines;
      boost::split(lines, data, boost::is_any_of("\n"));
      for (size_t i = 0; i < lines.size(); i++) {
        boost::smatch str_matches;
        static const boost::regex nmbr_regex("^(\\S+)\\s*=\\s*(\\S+)\\s*$");
        if (boost::regex_match(lines[i], str_matches, nmbr_regex)) {
          std::string key = str_matches[1];
          std::string value = str_matches[2];
          Logger::debug("found pair %s=%s", key.c_str(), value.c_str());

          if (key == "NMBR") {
            // TODO value empty?
            std::string msg;
            bool block = isNumberBlocked(m_settings.blockMode, value, &msg);
            Logger::notice(msg.c_str());
            m_foundCID = true;
          }
        }
      }
    }
  }

  // detect ringing stopped
  if (m_numRings > 0) {
    // when is 7s are elapsed, when ringing each 5s a RING is expected
    time_t diff = getMonotonicTime() - m_ringTime;
    if (diff > 6) {
      Logger::debug("ringing stopped");
      m_numRings = m_ringTime = 0;
      m_foundCID = false;
      // TODO notice? YES
    }
  }
}

bool AnalogPhone::openDevice() {
  m_FD = open(m_settings.device.c_str(), O_RDWR|O_NOCTTY);
  if (m_FD < 0) {
    Logger::warn("failed to open device %s (%s)", m_settings.device.c_str(), strerror(errno));
    m_FD = -1;
    return false;
  }

  // get original options
  if (tcgetattr(m_FD, &m_origTermios) < 0) {
    Logger::warn("tcgetattr failed on device %s (%s)", m_settings.device.c_str(), strerror(errno));
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
    Logger::warn("tcgetattr failed on device %s (%s)", m_settings.device.c_str(), strerror(errno));
    return false;
  }

  return true;
}

bool AnalogPhone::sendCommand(std::string cmd) {
  Logger::debug("send %s command...", cmd.c_str());
  std::string sendCmd = cmd + "\r\n"; // CRLF

  int len = write(m_FD, sendCmd.c_str(), strlen(sendCmd.c_str()));
  if (len != strlen(sendCmd.c_str())) {
    Logger::warn("write command '%s' failed on device %s (%s)", cmd.c_str(), m_settings.device.c_str(), strerror(errno));
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
      if (strstr(buffer, "ERROR"))
      {
        Logger::warn("received 'ERROR' for command %s from device", cmd.c_str(), m_settings.device.c_str());
        ret = false;
        break;
      }
    }
  }
  buffer[size] = '\0';
  
  std::string str = buffer;
  boost::algorithm::trim(str);
  Logger::debug("received '%s' from device %s tries=%d ret=%d",
    str.c_str(), m_settings.device.c_str(), tries, ret);
  return ret;
}

bool AnalogPhone::getData(std::string* data) {
  bool res = false;
  struct pollfd pfd = {m_FD, POLLIN | POLLPRI, 0};
  int ret = poll(&pfd, 1, 50);  // timeout of 50ms
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
      boost::algorithm::trim(str);
      *data = str;
      Logger::debug("received data '%s' from device %s", data->c_str(), m_settings.device.c_str());
      res = true;
    }
  }
  return res;
}

