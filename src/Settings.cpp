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

#include "Settings.h" // API

#include <string>
#include <fstream>
#include <sstream>
#include <sys/inotify.h>
#include <json-c/json.h>
#include <pjsua-lib/pjsua.h>
#include <boost/algorithm/string/predicate.hpp>

#include "Logger.h"


Settings::Settings() : m_filename(SYSCONFDIR "/" PACKAGE_NAME "/settings.json"),
                       Notify(SYSCONFDIR "/" PACKAGE_NAME "/settings.json", IN_CLOSE_WRITE) {
  //m_filename = SYSCONFDIR "/" PACKAGE_NAME "/settings.json";
  load();
}

Settings::~Settings() {
  clear();
}

bool Settings::run() {
  if (hasChanged()) {
    Logger::info("reload %s", m_filename.c_str());
    clear();
    load();
    return true;
  }
  return false;
}

void Settings::clear() {
  m_analogPhones.clear();
  m_sipAccounts.clear();
}

bool Settings::load() {
  Logger::debug("loading file %s", m_filename.c_str());

  clear();

  std::ifstream in(m_filename.c_str());
  if (in.fail()) {
    Logger::warn("loading file %s failed", m_filename.c_str());
    return false;
  }

  std::stringstream buffer;
  buffer << in.rdbuf();
  std::string str = buffer.str();

  struct json_object* root = json_tokener_parse(str.c_str());

  std::string log_level;
  if (getObject(root, "log_level", &log_level)) {
    Logger::setLogLevel(log_level);
  }

  // Analog
  struct json_object* analog;
  if (json_object_object_get_ex(root, "analog", &analog)) {
    struct json_object* phones;
    if (json_object_object_get_ex(analog, "phones", &phones)) {
      for (size_t i = 0; i < json_object_array_length(phones); i++) {
        struct json_object* entry = json_object_array_get_idx(phones, i);
        bool enabled;
        if (!getObject(entry, "enabled", &enabled) || !enabled) {
          continue;
        }
        struct SettingAnalogPhone acc;
        if (!getBase(entry, &acc.base)) {
          continue;
        }
        if (!getObject(entry, "device", &acc.device)) {
          continue;
        }
        m_analogPhones.push_back(acc);
      }
    } else {
      Logger::debug("no <phones> section found in settings file %s", m_filename.c_str());
    }
  } else {
    Logger::debug("no <analog> section found in settings file %s", m_filename.c_str());
  }

  // SIP
  struct json_object* sip;
  if (json_object_object_get_ex(root, "sip", &sip)) {

    int pjsip_log_level;
    if (getObject(sip, "pjsip_log_level", &pjsip_log_level)) {
      pj_log_set_level(pjsip_log_level);
    }

    struct json_object* accounts;
    if (json_object_object_get_ex(sip, "accounts", &accounts)) {
      for (size_t i = 0; i < json_object_array_length(accounts); i++) {
        struct json_object* entry = json_object_array_get_idx(accounts, i);
        bool enabled;
        if (!getObject(entry, "enabled", &enabled) || !enabled) {
          continue;
        }
        struct SettingSipAccount acc;
        if (!getBase(entry, &acc.base)) {
          continue;
        }
        if (!getObject(entry, "from_domain", &acc.fromDomain)) {
          continue;
        }
        if (!getObject(entry, "from_username", &acc.fromUsername)) {
          continue;
        }
        if (!getObject(entry, "from_password", &acc.fromPassword)) {
          continue;
        }
        m_sipAccounts.push_back(acc);
      }
    } else {
      Logger::debug("no <accounts> section found in settings file %s", m_filename.c_str());
    }
  } else {
    Logger::debug("no <sip> section found in settings file %s", m_filename.c_str());
  }

  json_object_put(root); // free
  return true;
}

bool Settings::getObject(struct json_object* objbase, const char* objname, std::string* res) {
  struct json_object* n;
  
  if (!json_object_object_get_ex(objbase, objname, &n)) {
    Logger::warn("%s not found in settings file %s", objname, m_filename.c_str());
    return false;
  }
  if (json_object_get_type(n) != json_type_string) {
    Logger::warn("string type expected for %s in settings file %s", objname, m_filename.c_str());
    return false;
  }
  *res = json_object_get_string(n);
  return true;
}

bool Settings::getObject(struct json_object* objbase, const char* objname, int* res) {
  struct json_object* n;
  
  if (!json_object_object_get_ex(objbase, objname, &n)) {
    Logger::warn("%s not found in settings file %s", objname, m_filename.c_str());
    return false;
  }
  if (json_object_get_type(n) != json_type_int) {
    Logger::warn("string type expected for %s in settings file %s", objname, m_filename.c_str());
    return false;
  }
  *res = json_object_get_int(n);
  return true;
}

bool Settings::getObject(struct json_object* objbase, const char* objname, bool* res) {
  struct json_object* n;
  
  if (!json_object_object_get_ex(objbase, objname, &n)) {
    Logger::warn("%s not found in settings file %s", objname, m_filename.c_str());
    return false;
  }
  if (json_object_get_type(n) != json_type_boolean) {
    Logger::warn("string type expected for %s in settings file %s", objname, m_filename.c_str());
    return false;
  }
  *res = (bool)json_object_get_boolean(n);
  return true;
}

bool Settings::getBase(struct json_object* objbase, struct SettingBase* res) {
  std::string tmp;
  // name
  if (!getObject(objbase, "name", &res->name)) {
    return false;
  }
  // block mode
  if (!getObject(objbase, "block_mode", &tmp)) {
    return false;
  }
  if (tmp == "logging_only") res->blockMode = LOGGING_ONLY;
  else if (tmp == "whitelists_only") res->blockMode = WHITELISTS_ONLY;
  else if (tmp == "whitelists_and_blacklists") res->blockMode = WHITELISTS_AND_BLACKLISTS;
  else if (tmp == "blacklists_only") res->blockMode = BLACKLISTS_ONLY;
  else {
    Logger::warn("unknown block mode '%s' in settings file %s", tmp.c_str(), m_filename.c_str());
    return false;
  }
  // counry code
  if (!getObject(objbase, "country_code", &tmp)) {
    return false;
  }
  if (!boost::starts_with(tmp, "+")) {
    Logger::warn("invalid country code'%s' in settings file %s", tmp.c_str(), m_filename.c_str());
    return false;
  }
  res->countryCode = tmp;
  return true;
}

void Settings::dump() {
  // TODO?
}

