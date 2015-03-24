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
#include <boost/algorithm/string/predicate.hpp>

#include "Logger.h"


/*
  AT commands:
  http://support.usr.com/support/5637/5637-ug/ref_data.html
*/
#define AT_Z_STR        "AT Z S0=0 E0 Q0 V1"
#define AT_CID_STR      "AT+VCID=1"
#define AT_HANGUP_STR   "ATH0"
#define AT_PICKUP_STR   "ATH1"


static time_t getMonotonicTime(void) {
  struct timespec tp;
  (void)clock_gettime(CLOCK_MONOTONIC, &tp);
  return tp.tv_sec;
}


AnalogPhone::AnalogPhone(Block* pBlock) : Phone(pBlock) {
  m_numRings = m_ringTime = 0;
  m_foundCID = false;
}

AnalogPhone::~AnalogPhone() {
  Logger::debug("~AnalogPhone...");
}

bool AnalogPhone::init(struct SettingAnalogPhone* phone) {
  Logger::debug("AnalogPhone::init(%s)...", phone->toString().c_str());
  m_settings = *phone; // struct copy

  if (!m_modem.open(phone->device)) {
    return false;
  }

  if (!m_modem.sendCommand(AT_Z_STR)) {
    return false;
  }
  if (!m_modem.sendCommand(AT_CID_STR)) {
    return false;
  }
  return true;
}

// load this into a seperate thread, needed for LiveAPI access, which may take some time...,
// or offload LiveAPI access itself into a seperate thread? YES?
void AnalogPhone::run() {
  std::string data;
  if (m_modem.getData(&data)) {
    if (data == "RING") {
      m_numRings++;
      m_ringTime = getMonotonicTime();

      if (m_numRings == 1) {
        Logger::debug("State changed to RINGING");
      }

      // handle no caller ID
      if (m_numRings == 2) {
        if (!m_foundCID) {
          Logger::warn("Not expecting second RING without caller ID");
        }
      }
    } else {
      // between first and second RING:
      // DATE=0306
      // TIME=1517
      // NMBR=0123456789
      // NAME=aasdasdd

      bool block = false;
      std::vector<std::string> lines;
      boost::split(lines, data, boost::is_any_of("\n"));
      for (size_t i = 0; i < lines.size(); i++) {
        boost::smatch str_matches;
        static const boost::regex nmbr_regex("^(\\S+)\\s*=\\s*(\\S+)\\s*$");
        if (boost::regex_match(lines[i], str_matches, nmbr_regex)) {
          std::string key = str_matches[1];
          std::string value = str_matches[2];
          //Logger::debug("found pair %s=%s", key.c_str(), value.c_str());

          if (key == "NMBR") {
            m_foundCID = true;
            std::string number = value;

            if (number == "PRIVATE") {
              // Caller ID information has been blocked by the user at the other end
              // see http://ads.usr.com/support/3453c/3453c-ug/dial_answer.html#IDfunctions
              std::string msg;
              block = isUnknownNumberBlocked(&m_settings.base, &msg);
              Logger::notice(msg.c_str());
              break;
            }

            // make number international
            if (boost::starts_with(number, "00")) number = "+" + number.substr(2);
            else if (boost::starts_with(number, "0")) number = m_settings.base.countryCode + number.substr(1);

            std::string msg;
            block = isNumberBlocked(&m_settings.base, number, &msg);
            Logger::notice(msg.c_str());
            break;
          }
        }
      } // for    

      if (block) {
        m_modem.sendCommand("ATH0"); // pickup
        m_modem.sendCommand("ATH1"); // hangup
        Logger::debug("State changed to HANGUP");
      }
    }
  }

  // detect ringing stopped
  if (m_numRings > 0) {
    // when is 7s are elapsed, when ringing each 5s a RING is expected
    time_t diff = getMonotonicTime() - m_ringTime;
    if (diff > 6) {
      Logger::debug("State changed to RINGING_STOPPED");
      m_numRings = m_ringTime = 0;
      m_foundCID = false;
    }
  }
}

