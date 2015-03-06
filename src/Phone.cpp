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

#include "Phone.h" // API

#include "Logger.h"


Phone::Phone(Lists* whitelists, Lists* blacklists) {
  m_whitelists = whitelists;
  m_blacklists = blacklists;
}

Phone::~Phone() {
  Logger::debug("~Phone...");
  m_whitelists = NULL;
  m_blacklists = NULL;
}

bool Phone::isNumberBlocked(enum SettingBlockMode blockMode, const std::string& number, std::string* msg) {
  std::string reason;
  bool block;
  switch (blockMode) {
    default:
      Logger::warn("invalid block mode %d", blockMode);
    case LOGGING_ONLY:
      reason = "";
      block = false;
      break;

    case WHITELISTS_ONLY:
      if (m_whitelists->isListed(number)) {
        reason = "found in whitelist";
        block = false;
      } else {
        reason = "";
        block = true;
      }
      break;

    case WHITELISTS_AND_BLACKLISTS:
      if (m_whitelists->isListed(number)) {
        reason = "found in whitelist";
        block = false;
      } else {
        reason = "found in blacklist";
        block = m_blacklists->isListed(number); 
      }
      break;

    case BLACKLISTS_ONLY:
      if (m_blacklists->isListed(number)) {
        reason = "found in blacklist";
        block = true;
      } else {
        reason = "";
        block = false;
      }
  }

  std::string res = "Incoming call from ";
  res += number;
  if (block) {
    res += " is blocked";
  }
  if (reason.length() > 0) {
    res += " (";
    res += reason;
    res += ")";
  }
  *msg = res;
  return block;
}

