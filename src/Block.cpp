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

#include "Block.h" // API

#include <sstream>
#include <json-c/json.h>

#include "Logger.h"
#include "Utils.h"


Block::Block(Settings* pSettings) {
  Logger::debug("Block::Block()...");
  m_pSettings = pSettings;

  m_pWhitelists = new FileListsNotified(Utils::pathJoin(pSettings->getBasePath(), "whitelists"));
  m_pBlacklists = new FileListsNotified(Utils::pathJoin(pSettings->getBasePath(), "blacklists"));
  m_pCache = new FileListsCache(Utils::pathJoin(pSettings->getBasePath(), "cache"));
}

Block::~Block() {
  Logger::debug("Block::~Block()...");

  delete(m_pCache);
  delete(m_pBlacklists);
  delete(m_pWhitelists);
}

void Block::run() {
  m_pWhitelists->run();
  m_pBlacklists->run();
  m_pCache->run();
}

bool Block::isBlocked(const struct SettingBase* pSettings, const std::string& rNumber, std::string* pMsg) {
  Logger::debug("Block::isBlocked(%s,number=%s)", Settings::toString(pSettings).c_str(), rNumber.c_str());

  if (rNumber == BLOCK_ANONYMOUS_NUMBER_STR) {
    return isAnonymousNumberBlocked(pSettings, pMsg);
  }

  std::string e164Number = rNumber;
  bool validNumber = true;
  Utils::makeNumberE164(pSettings, &e164Number, &validNumber);

  return isNumberBlocked(pSettings, e164Number, validNumber, pMsg);
}

bool Block::isAnonymousNumberBlocked(const struct SettingBase* pSettings, std::string* pMsg) {
  bool block = false;
  switch (pSettings->blockMode) {
    default:
      Logger::warn("Invalid block mode %d", pSettings->blockMode);
    case LOGGING_ONLY:
      block = false;
      break;
    case WHITELISTS_ONLY:
      block = true;
      break;
    case WHITELISTS_AND_BLACKLISTS:
    case BLACKLISTS_ONLY:
      block = pSettings->blockAnonymousCID;
      break;
  }

  // Incoming call number='anonymous' [blocked]
  std::ostringstream oss;
  oss << "Incoming call: number='anonymous'";
  if (block) {
    oss << " blocked";
  }

  *pMsg = oss.str();
  return block;
}

bool Block::isNumberBlocked(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber, std::string* pMsg) {
  std::string listName = "";
  std::string callerName = "";
  std::string score = "";
  bool onWhitelist = false;
  bool onBlacklist = false;
  bool block = false;
  
  switch (pSettings->blockMode) {
    default:
      Logger::warn("Invalid block mode %d", pSettings->blockMode);
    case LOGGING_ONLY:
      if (isWhiteListed(pSettings, rNumber, &listName, &callerName)) {
        onWhitelist = true;
        break;
      }
      if (isBlacklisted(pSettings, rNumber, validNumber, &listName, &callerName, &score)) {
        onBlacklist = true;
        break;
      }
      break;
    case WHITELISTS_ONLY:
      if (isWhiteListed(pSettings, rNumber, &listName, &callerName)) {
        onWhitelist = true;
        break;
      }
      block = true;
      break;
    case WHITELISTS_AND_BLACKLISTS:
      if (isWhiteListed(pSettings, rNumber, &listName, &callerName)) {
        onWhitelist = true;
        break;
      }
      if (isBlacklisted(pSettings, rNumber, validNumber, &listName, &callerName, &score)) {
        onBlacklist = true;
        block = true;
        break;
      }
      if (!validNumber && pSettings->blockInvalidCID) {
        block = true;
        break;
      }
      break;
    case BLACKLISTS_ONLY:
      if (isBlacklisted(pSettings, rNumber, validNumber, &listName, &callerName, &score)) {
        onBlacklist = true;
        block = true;
        break;
      }
      if (!validNumber && pSettings->blockInvalidCID) {
        block = true;
        break;
      }
      break;
  }

  if (!onWhitelist && !onBlacklist) {
    // online lookup caller name
    onlineLookup(pSettings, rNumber, validNumber, &callerName);
  }

  // Incoming call number='x' name='y' [invalid] [blocked] [whitelist='w'] [blacklist='b'] [score=s]
  std::ostringstream oss;
  oss << "Incoming call: number='" << rNumber << "'";
  if (callerName.length() != 0) {
    oss << " name='" << Utils::escapeSqString(callerName) << "'";
  }
  if (block) {
    oss << " blocked";
  }
  if (!validNumber) {
    oss << " invalid";
  }
  if (onWhitelist) {
    oss << " whitelist='" << listName << "'";
  }
  if (onBlacklist) {
    oss << " blacklist='" << listName << "'";
  }
  if (score.length() != 0) {
    oss << " score=" << score;
  }

  *pMsg = oss.str();
  return block;
}

bool Block::isWhiteListed(const struct SettingBase* pSettings, const std::string& rNumber,
                          std::string* pListName, std::string* pCallerName) {
  return m_pWhitelists->getEntry(rNumber, pListName, pCallerName);
}

bool Block::isBlacklisted(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber,
                          std::string* pListName, std::string* pCallerName, std::string* pScore) {
  if (m_pBlacklists->getEntry(rNumber, pListName, pCallerName)) {
    return true;
  }

  // online check if spam
  if (pSettings->onlineCheck.length() != 0) {

    if (pSettings->onlineCache) {
      // in cache?
      if (m_pCache->getEntry(CacheType::OnlineCheck, rNumber, pCallerName)) {
        *pListName = "cache";
        return true;
      }
    }

    struct json_object* root;
    if (checkOnline("onlinecheck_", pSettings->onlineCheck, rNumber, validNumber, &root)) {
      bool spam;
      if (!Utils::getObject(root, "spam", true, "script result", &spam, false)) {
        return false;
      }
      if (spam) {
        *pListName = pSettings->onlineCheck;
        (void)Utils::getObject(root, "name", false, "script result", pCallerName, "");
        int score;
        if (Utils::getObject(root, "score", false, "script result", &score, 0)) {
          *pScore = std::to_string(score);
        }

        if (pSettings->onlineCache) {
          // it is spam -> add to cache
          std::string cacheName = "from '" + pSettings->onlineCheck + "'";
          if (pCallerName->length() != 0) {
            cacheName += ": " + *pCallerName;
          }
          if (pScore->length() != 0) {
            cacheName += " with score '" + *pScore + "'";
          }
          m_pCache->addEntry(CacheType::OnlineCheck, rNumber, cacheName);
        }

        return true;
      }
    }
  }

  // no spam
  return false;
}

void Block::onlineLookup(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber,
                         std::string* pCallerName) {
  if (pSettings->onlineCache) {
    // in cache?
    if (m_pCache->getEntry(CacheType::OnlineLookup, rNumber, pCallerName)) {
      return;
    }
  }

  if (pSettings->onlineLookup.length() != 0) {
    struct json_object* root;
    if (checkOnline("onlinelookup_", pSettings->onlineLookup, rNumber, validNumber, &root)) {
      (void)Utils::getObject(root, "name", false, "script result", pCallerName, "");

      if (pSettings->onlineCache) {
        if (pCallerName->length() != 0) {
          // found CID -> add to cache
          m_pCache->addEntry(CacheType::OnlineLookup, rNumber, *pCallerName);
        }
      }
    }
  }
}

bool Block::checkOnline(std::string prefix, std::string scriptBaseName, const std::string& rNumber, const bool validNumber,
                        struct json_object** root) {
  if (Utils::startsWith(rNumber, "**")) {
    // it is an intern number, thus makes no sense to ask the world
    return false;
  }
  if (!validNumber) {
    // number is invalid, thus makes no sense to ask the world
    return false;
  }

  std::string script = DATADIR "/callblocker/" + prefix + scriptBaseName + ".py";
  std::string parameters = "--number " + rNumber;
  std::vector<struct SettingOnlineCredential> creds = m_pSettings->getOnlineCredentials();
  for(size_t i = 0; i < creds.size(); i++) {
    struct SettingOnlineCredential* cred = &creds[i];
    if (cred->name == scriptBaseName) {
      for (std::map<std::string,std::string>::iterator it = cred->data.begin(); it != cred->data.end(); ++it) {
        parameters += " --" + it->first + "=" + it->second;
      }
      break;
    }
  }

  std::string res;
  if (!Utils::executeCommand(script + " " + parameters + " 2>&1", &res)) {
    return false; // script failed, error already logged
  }

  *root = json_tokener_parse(res.c_str());
  return true; // script executed successful
}

