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

#include "Settings.h" // API

#include <fstream>
#include <json-c/json.h>
#include <pjsua-lib/pjsua.h>
#include <sstream>
#include <string>
#include <sys/inotify.h>

#include "Logger.h"
#include "Utils.h"

Settings::Settings(const std::string& rPathname)
    : Notify(rPathname, IN_CLOSE_WRITE)
{
    Logger::debug("Settings::Settings(rPathname='%s')", rPathname.c_str());
    m_basePathname = rPathname;
    m_filename = rPathname + "/settings.json";
    load();
}

Settings::~Settings()
{
    Logger::debug("Settings::~Settings() of '%s'", m_filename.c_str());
    clear();
}

bool Settings::hasChanged()
{
    if (Notify::hasChanged()) {
        Logger::info("reload settings");
        clear();
        load();
        return true;
    }
    return false;
}

std::string Settings::getBasePath()
{
    return m_basePathname;
}

void Settings::clear()
{
    m_analogPhones.clear();
    m_sipAccounts.clear();
}

bool Settings::load()
{
    Logger::debug("Settings::load() of '%s'", m_filename.c_str());
    clear();

    struct json_object* root;
    if (!Utils::loadJson(m_filename, &root)) {
        return false;
    }

    std::string strLogLevel;
    (void)Utils::getObject(root, "log_level", true, m_filename, &strLogLevel, "info");
    LogLevel logLevel = LogLevel::INFO;
    if (strLogLevel == "debug")
        logLevel = LogLevel::DEBUG;
    else if (strLogLevel == "info")
        logLevel = LogLevel::INFO;
    else if (strLogLevel == "notice")
        logLevel = LogLevel::NOTICE;
    else if (strLogLevel == "warn")
        logLevel = LogLevel::WARN;
    else if (strLogLevel == "error")
        logLevel = LogLevel::ERROR;
    else
        Logger::warn("ignore invalid log_level %s", strLogLevel.c_str());
    (void)Logger::setLogLevel(logLevel);

    int pjsip_log_level;
    if (Utils::getObject(root, "pjsip_log_level", false, m_filename, &pjsip_log_level, 0)) {
        pj_log_set_level(pjsip_log_level);
    }

    // Phones
    struct json_object* phones;
    if (json_object_object_get_ex(root, "phones", &phones)) {
        for (int i = 0; i < json_object_array_length(phones); i++) {
            struct json_object* entry = json_object_array_get_idx(phones, i);
            bool enabled;
            if (!Utils::getObject(entry, "enabled", true, m_filename, &enabled, false) || !enabled) {
                continue;
            }

            if (json_object_object_get_ex(entry, "device", nullptr)) {
                // Analog account
                struct SettingAnalogPhone analog;
                if (!getBase(entry, &analog.base)) {
                    continue;
                }
                if (!Utils::getObject(entry, "device", true, m_filename, &analog.device, "")) {
                    continue;
                }
                m_analogPhones.push_back(analog);
            } else {
                // SIP account
                struct SettingSipAccount sip;
                if (!getBase(entry, &sip.base)) {
                    continue;
                }
                if (!Utils::getObject(entry, "domain", true, m_filename, &sip.domain, "")) {
                    continue;
                }
                if (!Utils::getObject(entry, "username", true, m_filename, &sip.username, "")) {
                    continue;
                }
                if (!Utils::getObject(entry, "password", true, m_filename, &sip.password, "")) {
                    continue;
                }
                // optionals
                (void)Utils::getObject(entry, "realm", false, m_filename, &sip.realm, sip.domain);
                (void)Utils::getObject(entry, "outbound_proxy", false, m_filename, &sip.outboundProxy, "");
                (void)Utils::getObject(entry, "secure", false, m_filename, &sip.secure, false);
                (void)Utils::getObject(entry, "forceIPv4", false, m_filename, &sip.forceIPv4, true);

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
            if (!Utils::getObject(entry, "name", true, m_filename, &cred.name, "")) {
                continue;
            }
            json_object_object_foreach(entry, key, value)
            {
                (void)value; // not used here
                if (strcmp("name", key) == 0)
                    continue;
                std::string value_str;
                if (!Utils::getObject(entry, key, true, m_filename, &value_str, "")) {
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

bool Settings::getBase(struct json_object* objbase, struct SettingBase* res)
{
    // name
    if (!Utils::getObject(objbase, "name", true, m_filename, &res->name, "")) {
        return false;
    }
    // country code
    if (!Utils::getObject(objbase, "country_code", true, m_filename, &res->countryCode, "")) {
        return false;
    }
    if (!Utils::startsWith(res->countryCode, "+")) {
        Logger::warn("invalid country_code '%s' in settings file %s", res->countryCode.c_str(), m_filename.c_str());
        return false;
    }
    // block mode
    std::string tmp;
    if (!Utils::getObject(objbase, "block_mode", true, m_filename, &tmp, "logging_only")) {
        return false;
    }
    if (tmp == "logging_only")
        res->blockMode = LOGGING_ONLY;
    else if (tmp == "allowlists_only")
        res->blockMode = ALLOWLISTS_ONLY;
    else if (tmp == "allowlists_and_blocklists")
        res->blockMode = ALLOWLISTS_AND_BLOCKLISTS;
    else if (tmp == "blocklists_only")
        res->blockMode = BLOCKLISTS_ONLY;
    else {
        Logger::warn("unknown block_mode '%s' in settings file %s", tmp.c_str(), m_filename.c_str());
        return false;
    }

    // optionals
    (void)Utils::getObject(objbase, "block_anonymous_cid", false, m_filename, &res->blockAnonymousCID, false);
    (void)Utils::getObject(objbase, "block_invalid_cid", false, m_filename, &res->blockInvalidCID, true);
    (void)Utils::getObject(objbase, "online_check", false, m_filename, &res->onlineCheck, "");
    (void)Utils::getObject(objbase, "online_lookup", false, m_filename, &res->onlineLookup, "");
    (void)Utils::getObject(objbase, "online_cache", false, m_filename, &res->onlineCache, true);

    return true;
}

std::string Settings::toString(const struct SettingBase* pBase)
{
    std::ostringstream oss;
    oss << "n=" << pBase->name
        << ",cc=" << pBase->countryCode
        << ",bm=" << pBase->blockMode << ",bacid=" << pBase->blockAnonymousCID << ",bicid=" << pBase->blockInvalidCID
        << ",on=" << pBase->onlineCheck << ",ol=" << pBase->onlineLookup;
    return oss.str();
}

std::string Settings::toString(const struct SettingSipAccount* pSip)
{
    std::ostringstream oss;
    oss << Settings::toString(&pSip->base)
        << ",d=" << pSip->domain
        << ",u=" << pSip->username
        << ",r=" << pSip->realm
        << ",op=" << pSip->outboundProxy
        << ",s=" << pSip->secure
        << ",ip4=" << pSip->forceIPv4;
    return oss.str();
}

std::string Settings::toString(const struct SettingAnalogPhone* pAnalog)
{
    std::ostringstream oss;
    oss << Settings::toString(&pAnalog->base)
        << ",d=" << pAnalog->device;
    return oss.str();
}
