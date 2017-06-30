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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>
#include <vector>
#include <map>

#include "Notify.h"

// forward declaration
struct json_object; // avoids inclusion of <json-c/json.h>


enum SettingBlockMode {
  LOGGING_ONLY = 0,             // number is never blocked, only logged what it would do
  WHITELISTS_ONLY,              // number is blocked, when NOT in a whitelists (blacklists not used at all)
  WHITELISTS_AND_BLACKLISTS,    // number is blocked, when in a blacklists and NOT in a whitelists
  BLACKLISTS_ONLY               // number is blocked, when in a blacklists (whitelists not used at all)
};

struct SettingBase {
  std::string name;
  std::string countryCode;

  enum SettingBlockMode blockMode;
  bool blockAnonymousCID;
  bool blockInvalidCID;

  std::string onlineCheck;
  std::string onlineLookup;
  bool onlineCache;
};

struct SettingSipAccount {
  struct SettingBase base;
  std::string domain;
  std::string username;
  std::string password;
  std::string realm;
  bool secure;
  bool forceIPv4;
};

struct SettingAnalogPhone {
  struct SettingBase base;
  std::string device;
};

struct SettingOnlineCredential {
  std::string name;
  std::map<std::string, std::string> data;
};

class Settings : public Notify {
private:
  std::string m_basePathname; // default: /etc/callblocker
  std::string m_filename;     // full filename for settings.json
  std::vector<struct SettingSipAccount> m_sipAccounts;
  std::vector<struct SettingAnalogPhone> m_analogPhones;
  std::vector<struct SettingOnlineCredential> m_onlineCredentials;

public:
  Settings(const std::string& rPathname);
  virtual ~Settings();
  virtual bool hasChanged();

  std::string getBasePath();

  std::vector<struct SettingSipAccount> getSipAccounts() { return m_sipAccounts; }
  std::vector<struct SettingAnalogPhone> getAnalogPhones() { return m_analogPhones; }
  std::vector<struct SettingOnlineCredential> getOnlineCredentials() { return m_onlineCredentials; }
  
  static std::string toString(const struct SettingBase* pBase);
  static std::string toString(const struct SettingSipAccount* pSip);
  static std::string toString(const struct SettingAnalogPhone* pAnalog);

private:
  void clear();
  bool load();
  bool getBlockMode(struct json_object* objbase, enum SettingBlockMode* res);
  bool getBase(struct json_object* objbase, struct SettingBase* res);
};

#endif

