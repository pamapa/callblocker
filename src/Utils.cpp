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

#include "Utils.h" // API

#include <string.h>
#include <sstream>

#include "Logger.h"


bool Utils::getObject(struct json_object* objbase, const char* objname, bool logError, const std::string& rLocation, std::string* pRes) {
  struct json_object* n;
  *pRes = "";
  if (!json_object_object_get_ex(objbase, objname, &n)) {
    if (logError) Logger::warn("%s not found in %s", objname, rLocation.c_str());
    return false;
  }
  if (json_object_get_type(n) != json_type_string) {
    if (logError) Logger::warn("string type expected for %s in %s", objname, rLocation.c_str());
    return false;
  }
  *pRes = json_object_get_string(n);
  return true;
}

bool Utils::getObject(struct json_object* objbase, const char* objname, bool logError, const std::string& rLocation, int* pRes) {
  struct json_object* n;
   *pRes = 0;
  if (!json_object_object_get_ex(objbase, objname, &n)) {
    if (logError) Logger::warn("%s not found in %s", objname, rLocation.c_str());
    return false;
  }
  if (json_object_get_type(n) != json_type_int) {
    if (logError) Logger::warn("integer type expected for %s in %s", objname, rLocation.c_str());
    return false;
  }
  *pRes = json_object_get_int(n);
  return true;
}

bool Utils::getObject(struct json_object* objbase, const char* objname, bool logError, const std::string& rLocation, bool* pRes) {
  struct json_object* n;
  *pRes = false;
  if (!json_object_object_get_ex(objbase, objname, &n)) {
    if (logError) Logger::warn("%s not found in %s", objname, rLocation.c_str());
    return false;
  }
  if (json_object_get_type(n) != json_type_boolean) {
    if (logError) Logger::warn("boolean type expected for %s in %s", objname, rLocation.c_str());
    return false;
  }
  *pRes = (bool)json_object_get_boolean(n);
  return true;
}

bool Utils::executeCommand(const std::string& rCmd, std::string* pRes) {
  Logger::debug("executing(%s)...", rCmd.c_str());

  FILE* fp = popen(rCmd.c_str(), "r");
  if (fp == NULL) {
    Logger::warn("popen failed (%s)", strerror(errno));
    return false;
  }

  std::string res = "";
  char buf[128];
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    res += buf;
  }
  Utils::trim(res);

  int status = pclose(fp);
  if (status != 0) {
    Logger::warn("%s failed (%s)", rCmd.c_str(), res.c_str());
    return false;
  }

  Logger::debug("result: %s", res.c_str());
  *pRes = res;
  return true;
}

std::string Utils::getPjStatusAsString(pj_status_t status) {
  static char buf[100];
  pj_str_t pjstr = pj_strerror(status, buf, sizeof(buf)); 
  std::string ret = pj_strbuf(&pjstr);
  return ret;
}

bool Utils::startsWith(const std::string& rStr, const char* pPrefix) {
  return rStr.find(pPrefix) == 0 ? true : false;
}

static void leftTrim(std::string& rStr) {
  std::string::size_type pos = rStr.find_last_not_of(" \t\r\n");
  if (pos != std::string::npos) {
    rStr.erase(pos + 1);
  }
}

static void rightTrim(std::string& rStr) {
  std::string::size_type pos = rStr.find_first_not_of(" \t\r\n");
  if (pos != std::string::npos) {
    rStr.erase(0, pos);
  }
}

void Utils::trim(std::string& rStr) {
  leftTrim(rStr);
  rightTrim(rStr);
}

std::string Utils::getBaseFilename(const std::string& rFilename) {
  std::string::size_type pos = rFilename.find_last_of("/");
  if (pos != std::string::npos) return rFilename.substr(pos + 1);
  else return rFilename;
}

std::string Utils::escapeSqString(const std::string& rStr) {
  std::size_t n = rStr.length();
  std::string escaped;
  escaped.reserve(n * 2); // pessimistic preallocation
  for (std::size_t i = 0; i < n; ++i) {
    if (rStr[i] == '\\' || rStr[i] == '\'') {
      escaped += '\\';
    }
    escaped += rStr[i];
  }
  return escaped;
}

std::string Utils::makeNumberInternational(const struct SettingBase* pSettings, const std::string& rNumber) {
  std::string res;
  if (Utils::startsWith(rNumber, "00")) res = "+" + rNumber.substr(2);
  else if (Utils::startsWith(rNumber, "0")) res = pSettings->countryCode + rNumber.substr(1);
  else res = rNumber;
  return res;
}

void Utils::parseCallerID(std::string& rData, std::vector<std::pair<std::string, std::string>>* pResult) {
  // DATE=0306
  // TIME=1517
  // NMBR=0123456789
  // NAME=aasdasdd

  // split by newline
  std::stringstream ss(rData);
  std::string to;
  while (std::getline(ss, to, '\n')) {
    // key=val
    std::string::size_type pos = to.find('=');
    if (pos == std::string::npos) continue;
    std::string key = to.substr(0, pos);
    std::string val = "";
    if (pos + 1 < to.length()) {
      val = to.substr(pos + 1, to.length() - pos);
    }
    Utils::trim(key);
    Utils::trim(val);
    std::pair<std::string, std::string> p(key, val);
    pResult->push_back(p);
  }
}

