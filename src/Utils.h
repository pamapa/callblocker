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

#ifndef UTILS_H
#define UTILS_H

#include <string>

#include "Settings.h"

// forward declaration
struct json_object;       // avoids inclusion of <json-c/json.h>
typedef int pj_status_t;  // avoids inclusion of <pjsua-lib/pjsua.h>


class Utils {
public:
  static bool getObject(struct json_object* objbase, const char* objname, bool logError, const std::string& rLocation, std::string* pRes);
  static bool getObject(struct json_object* objbase, const char* objname, bool logError, const std::string& rLocation, int* pRes);
  static bool getObject(struct json_object* objbase, const char* objname, bool logError, const std::string& rLocation, bool* pRes);

  static bool executeCommand(const std::string& rCmd, std::string* pRes);

  static std::string getPjStatusAsString(pj_status_t status);

  // string
  static bool startsWith(const std::string& rStr, const char* pPrefix);
  static void trim(std::string* pStr);
  static std::string getBaseFilename(const std::string& rFilename);
  static std::string escapeSqString(const std::string& rStr);

  // returns phone number in E.164 format
  static void makeNumberInternational(const struct SettingBase* pSettings, std::string* pNumber, bool* pValid);
  static void parseCallerID(std::string& rData, std::vector<std::pair<std::string, std::string> >* pResult);
};

#endif

