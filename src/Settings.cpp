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
#include "Helper.h"


Settings::Settings() : Notify(SYSCONFDIR "/" PACKAGE_NAME, IN_CLOSE_WRITE) {
  m_filename = SYSCONFDIR "/" PACKAGE_NAME "/settings.json";
  load();
}

Settings::~Settings() {
  clear();
}

bool Settings::hasChanged() {
  if (Notify::hasChanged()) {
    Logger::info("reload settings");
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
  if (Helper::getObject(root, "log_level", true, m_filename, &log_level)) {
    Logger::setLogLevel(log_level);
  }

  Logger::debug("loading file %s", m_filename.c_str());

  int pjsip_log_level;
  if (Helper::getObject(root, "pjsip_log_level", false, m_filename, &pjsip_log_level)) {
    pj_log_set_level(pjsip_log_level);
  }

  // Phones
  struct json_object* phones;
  if (json_object_object_get_ex(root, "phones", &phones)) {
    for (int i = 0; i < json_object_array_length(phones); i++) {
      struct json_object* entry = json_object_array_get_idx(phones, i);
      bool enabled;
      if (!Helper::getObject(entry, "enabled", true, m_filename, &enabled) || !enabled) {
        continue;
      }

      if (json_object_object_get_ex(entry, "device", NULL)) {
        // Analog
        struct SettingAnalogPhone analog;
        if (!getBase(entry, &analog.base)) {
          continue;
        }
        if (!Helper::getObject(entry, "device", true, m_filename, &analog.device)) {
          continue;
        }
        m_analogPhones.push_back(analog);
      } else {
        // SIP
        struct SettingSipAccount sip;
        if (!getBase(entry, &sip.base)) {
          continue;
        }
        if (!Helper::getObject(entry, "from_domain", true, m_filename, &sip.fromDomain)) {
          continue;
        }
        if (!Helper::getObject(entry, "from_username", true, m_filename, &sip.fromUsername)) {
          continue;
        }
        if (!Helper::getObject(entry, "from_password", true, m_filename, &sip.fromPassword)) {
          continue;
        }
        m_sipAccounts.push_back(sip);
      }
    }
  } else {
    Logger::warn("no <phones> section found in settings file %s", m_filename.c_str());
  }

  // credentials
  struct json_object* onlineCredentials;
  if (json_object_object_get_ex(root, "online_credentials", &onlineCredentials)) {
    for (int i = 0; i < json_object_array_length(onlineCredentials); i++) {
      struct json_object* entry = json_object_array_get_idx(onlineCredentials, i);

      struct SettingOnlineCredential cred;
      if (!Helper::getObject(entry, "name", true, m_filename, &cred.name)) {
        continue;
      }
      json_object_object_foreach(entry, key, value) {
        (void)value; // not used here
        if (strcmp("name", key) == 0) continue;
        std::string value_str;
        if (!Helper::getObject(entry, key, true, m_filename, &value_str)) {
          continue;
        }
        cred.data[key] = value_str;
      }
      m_onlineCredentials.push_back(cred);
    }
  } else {
    Logger::warn("no <online_credentials> section found in settings file %s", m_filename.c_str());
  }

  json_object_put(root); // free
  return true;
}

bool Settings::getBase(struct json_object* objbase, struct SettingBase* res) {
  // name
  if (!Helper::getObject(objbase, "name", true, m_filename, &res->name)) {
    return false;
  }
  // country code
  if (!Helper::getObject(objbase, "country_code", true, m_filename, &res->countryCode)) {
    return false;
  }
  if (!boost::starts_with(res->countryCode, "+")) {
    Logger::warn("invalid country_code '%s' in settings file %s", res->countryCode.c_str(), m_filename.c_str());
    return false;
  }
  // block mode
  std::string tmp;
  if (!Helper::getObject(objbase, "block_mode", true, m_filename, &tmp)) {
    return false;
  }
  if (tmp == "logging_only") res->blockMode = LOGGING_ONLY;
  else if (tmp == "whitelists_only") res->blockMode = WHITELISTS_ONLY;
  else if (tmp == "whitelists_and_blacklists") res->blockMode = WHITELISTS_AND_BLACKLISTS;
  else if (tmp == "blacklists_only") res->blockMode = BLACKLISTS_ONLY;
  else {
    Logger::warn("unknown block_mode '%s' in settings file %s", tmp.c_str(), m_filename.c_str());
    return false;
  }
  // block unknown cid
  (void)Helper::getObject(objbase, "block_anonymous_cid", false, m_filename, &res->blockAnonymousCID);
  // online check
  (void)Helper::getObject(objbase, "online_check", false, m_filename, &res->onlineCheck);
  // online check
  (void)Helper::getObject(objbase, "online_lookup", false, m_filename, &res->onlineLookup);

  return true;
}

void Settings::dump() {
  // TODO?
}

