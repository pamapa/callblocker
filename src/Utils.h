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

#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <chrono>
#include <sys/time.h>

#include "Settings.h"

// forward declaration
struct json_object;       // avoids inclusion of <json-c/json.h>
typedef int pj_status_t;  // avoids inclusion of <pjsua-lib/pjsua.h>


class Utils {
public:
  // file system
  static std::string pathJoin(const std::string& rPath, const std::string& rFilename);
  static std::string pathBasename(const std::string& rPath);
  static std::string pathDirname(const std::string& rPath);
  static std::string pathAbsname(const std::string& rPath);
  static bool pathExists(const std::string& rPath);
  static bool fileCopy(const std::string& rFrom, const std::string& rTo);

  // json (call json_object_put for free)
  static bool loadJson(const std::string& filename, struct json_object** pRoot);
  static bool parseJson(const std::string& str, struct json_object** pRoot);
  static bool getObject(struct json_object* objbase, const char* objname, bool logNotFoundError, const std::string& rLocation,
                        std::string* pRes, const std::string& rDefault);
  static bool getObject(struct json_object* objbase, const char* objname, bool logNotFoundError, const std::string& rLocation,
                        int* pRes, const int rDefault);
  static bool getObject(struct json_object* objbase, const char* objname, bool logNotFoundError, const std::string& rLocation,
                        bool* pRes, const bool rDefault);
  static bool getObject(struct json_object* objbase, const char* objname, bool logNotFoundError, const std::string& rLocation,
                        std::chrono::system_clock::time_point* pRes, const std::chrono::system_clock::time_point& rDefault);

  // execute
  static bool executeCommand(const std::string& rCmd, std::string* pRes);

  // string
  static std::string getPjStatusAsString(pj_status_t status);
  static bool startsWith(const std::string& rStr, const char* pPrefix);
  static void trim(std::string* pStr);
  static std::string escapeSqString(const std::string& rStr);
  static void replaceAll(std::string* pStr, const std::string& rFrom, const std::string& rTo);
  // wildcard: *: zero or multiple, ?: single character
  static bool matchWithWildcards(const std::string &rStr, const std::string& rWildcardPattern, bool caseSensitive = false);

  // phone number
  static void makeNumberE164(const struct SettingBase* pSettings, std::string* pNumber, bool* pValid);
  static void parseCallerID(std::string& rData, std::vector<std::pair<std::string, std::string> >* pResult);

  // time
  static std::string formatTime(const std::chrono::system_clock::time_point& rTp);
  static bool parseTime(const std::string& rStr, std::chrono::system_clock::time_point* pRes);

  // ip
  // ai_family: AF_UNSPEC (IPv4 or IPv6), AF_INET (force IPv4 address) or AF_INET6 (force IPv6)
  static bool resolveHostname(const std::string& rHostname, int ai_family, std::string* pRes);
};

#endif

