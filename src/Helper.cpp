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

#include "Helper.h" // API

#include <string.h>
#include <boost/algorithm/string.hpp>

#include "Logger.h"


bool Helper::getObject(struct json_object* objbase, const char* objname, const char* location, std::string* res) {
  struct json_object* n;
  
  if (!json_object_object_get_ex(objbase, objname, &n)) {
    Logger::warn("%s not found in %s", objname, location);
    return false;
  }
  if (json_object_get_type(n) != json_type_string) {
    Logger::warn("string type expected for %s in %s", objname, location);
    return false;
  }
  *res = json_object_get_string(n);
  return true;
}

bool Helper::getObject(struct json_object* objbase, const char* objname, const char* location, int* res) {
  struct json_object* n;
  
  if (!json_object_object_get_ex(objbase, objname, &n)) {
    Logger::warn("%s not found in %s", objname, location);
    return false;
  }
  if (json_object_get_type(n) != json_type_int) {
    Logger::warn("string type expected for %s in %s", objname, location);
    return false;
  }
  *res = json_object_get_int(n);
  return true;
}

bool Helper::getObject(struct json_object* objbase, const char* objname, const char* location, bool* res) {
  struct json_object* n;
  
  if (!json_object_object_get_ex(objbase, objname, &n)) {
    Logger::warn("%s not found in %s", objname, location);
    return false;
  }
  if (json_object_get_type(n) != json_type_boolean) {
    Logger::warn("string type expected for %s in %s", objname, location);
    return false;
  }
  *res = (bool)json_object_get_boolean(n);
  return true;
}

bool Helper::executeCommand(const std::string cmd, std::string* pRes) {
  Logger::debug("executing(%s)..", cmd.c_str());

  FILE* fp = popen(cmd.c_str(), "r");
  if (fp == NULL) {
    Logger::warn("popen failed (%s)", strerror(errno));
    return false;
  }

  std::string res = "";
  char buf[128];
  while (fgets(buf, sizeof(buf), fp) != NULL) {
    res += buf;
  }
  boost::algorithm::trim(res);
  *pRes = res;
  Logger::debug("res=%s", res.c_str());

  int status = pclose(fp);
  if (status != 0) {
    Logger::warn("%s failed (%i)", cmd.c_str(), status);
    return false;
  }
  return true;
}

std::string Helper::getPjStatusAsString(pj_status_t status) {
  static char buf[100];
  pj_str_t pjstr = pj_strerror(status, buf, sizeof(buf)); 
  std::string ret = pj_strbuf(&pjstr);
  return ret;
}

