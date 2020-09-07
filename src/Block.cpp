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

#include "Block.h" // API

#include <sstream>
#include <json-c/json.h>

#include "Logger.h"
#include "Utils.h"


Block::Block(Settings* pSettings) {
  Logger::debug("Block::Block()");
  m_pSettings = pSettings;

  m_pAllowlists = new FileListsNotified(Utils::pathJoin(pSettings->getBasePath(), "allowlists"));
  m_pBlocklists = new FileListsNotified(Utils::pathJoin(pSettings->getBasePath(), "blocklists"));
  m_pCache = new FileListsCache(Utils::pathJoin(pSettings->getBasePath(), "cache"));
}

Block::~Block() {
  Logger::debug("Block::~Block()");

  delete(m_pCache);
  delete(m_pBlocklists);
  delete(m_pAllowlists);
}

void Block::run() {
  m_pAllowlists->run();
  m_pBlocklists->run();
  m_pCache->run();
}

bool Block::isBlocked(const struct SettingBase* pSettings, const std::string& rNumber, const std::string& rName, std::string* pMsg) {
  Logger::debug("Block::isBlocked(pSettings='%s',rNumber='%s',rName='%s')", Settings::toString(pSettings).c_str(), rNumber.c_str(), rName.c_str());

  if (rNumber == BLOCK_ANONYMOUS_NUMBER_STR) {
    return isAnonymousNumberBlocked(pSettings, pMsg);
  }

  std::string e164Number = rNumber;
  bool validNumber = true;
  Utils::makeNumberE164(pSettings, &e164Number, &validNumber);

  return isNumberBlocked(pSettings, e164Number, validNumber, rName, pMsg);
}

bool Block::isAnonymousNumberBlocked(const struct SettingBase* pSettings, std::string* pMsg) {
  bool block = false;
  switch (pSettings->blockMode) {
    default:
      Logger::warn("Invalid block mode %d", pSettings->blockMode);
    case LOGGING_ONLY:
      block = false;
      break;
    case ALLOWLISTS_ONLY:
      block = true;
      break;
    case ALLOWLISTS_AND_BLOCKLISTS:
    case BLOCKLISTS_ONLY:
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

bool Block::isNumberBlocked(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber, const std::string& rName, std::string* pMsg) {
  std::string listName = "";
  std::string callerName = "";
  std::string score = "";
  bool onAllowlist = false;
  bool onBlocklist = false;
  bool block = false;
  
  switch (pSettings->blockMode) {
    default:
      Logger::warn("Invalid block mode %d, use LOGGING_ONLY", pSettings->blockMode);
      // fall-through
    case LOGGING_ONLY:
      if (isAllowListed(pSettings, rNumber, &listName, &callerName)) {
        onAllowlist = true;
        break;
      }
      if (isBlocklisted(pSettings, rNumber, validNumber, &listName, &callerName, &score)) {
        onBlocklist = true;
        break;
      }
      break;
    case ALLOWLISTS_ONLY:
      if (isAllowListed(pSettings, rNumber, &listName, &callerName)) {
        onAllowlist = true;
        break;
      }
      block = true;
      break;
    case ALLOWLISTS_AND_BLOCKLISTS:
      if (isAllowListed(pSettings, rNumber, &listName, &callerName)) {
        onAllowlist = true;
        break;
      }
      if (isBlocklisted(pSettings, rNumber, validNumber, &listName, &callerName, &score)) {
        onBlocklist = true;
        block = true;
        break;
      }
      if (!validNumber && pSettings->blockInvalidCID) {
        block = true;
        break;
      }
      break;
    case BLOCKLISTS_ONLY:
      if (isBlocklisted(pSettings, rNumber, validNumber, &listName, &callerName, &score)) {
        onBlocklist = true;
        block = true;
        break;
      }
      if (!validNumber && pSettings->blockInvalidCID) {
        block = true;
        break;
      }
      break;
  }

  if (callerName.empty() && !rName.empty()) {
    // use caller information from provider
    callerName = rName;
  }

  if (!onAllowlist && !onBlocklist && callerName.empty()) {
    // online lookup caller name
    onlineLookup(pSettings, rNumber, validNumber, &callerName);
  }

  // Incoming call number='x' name='y' [invalid] [blocked] [allowlist='w'] [blocklist='b'] [score=s]
  std::ostringstream oss;
  oss << "Incoming call: number='" << rNumber << "'";
  if (!callerName.empty()) {
    oss << " name='" << Utils::escapeSqString(callerName) << "'";
  }
  if (block) {
    oss << " blocked";
  }
  if (!validNumber) {
    oss << " invalid";
  }
  if (onAllowlist) {
    oss << " allowlist='" << listName << "'";
  }
  if (onBlocklist) {
    oss << " blocklist='" << listName << "'";
  }
  if (score.length() != 0) {
    oss << " score=" << score;
  }

  *pMsg = oss.str();
  return block;
}

bool Block::isAllowListed(const struct SettingBase* pSettings, const std::string& rNumber,
                          std::string* pListName, std::string* pCallerName) {
  return m_pAllowlists->getEntry(rNumber, pListName, pCallerName);
}

bool Block::isBlocklisted(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber,
                          std::string* pListName, std::string* pCallerName, std::string* pScore) {
  if (m_pBlocklists->getEntry(rNumber, pListName, pCallerName)) {
    return true;
  }

  // online check if spam
  if (onlineCheck(pSettings, rNumber, validNumber, pListName, pCallerName, pScore)) {
    return true;
  }

  // no spam
  return false;
}

void Block::onlineLookup(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber,
                         std::string* pCallerName) {
  if (pSettings->onlineCache) {
    // already in cache?
    if (m_pCache->getEntry(CacheType::OnlineLookup, rNumber, pCallerName)) {
      Logger::debug("Block::onlineLookup: found entry '%s' in cache -> '%s'", rNumber.c_str(), pCallerName->c_str());
      return;
    }
  }

  struct json_object* root;
  if (!executeScript("onlinelookup_", pSettings->onlineLookup, rNumber, validNumber, &root)) {
    return;
  }

  (void)Utils::getObject(root, "name", false, "script result", pCallerName, "");
  json_object_put(root); // free

  if (pSettings->onlineCache) {
    if (pCallerName->length() != 0) {
      // found CID -> add to cache
      m_pCache->addEntry(CacheType::OnlineLookup, rNumber, *pCallerName);
    }
  }
}

bool Block::onlineCheck(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber,
                        std::string* pListName, std::string* pCallerName, std::string* pScore) {
  if (pSettings->onlineCache) {
    // already in cache?
    if (m_pCache->getEntry(CacheType::OnlineCheck, rNumber, pCallerName)) {
      Logger::debug("Block::onlineCheck: found entry '%s' in cache -> its spam", rNumber.c_str());
      *pListName = "cache";
      return true;
    }
  }

  struct json_object* root;
  if (!executeScript("onlinecheck_", pSettings->onlineCheck, rNumber, validNumber, &root)) {
    // error already logged
    return false;
  }

  bool spam;
  if (!Utils::getObject(root, "spam", true, "script result", &spam, false)) {
    // error already logged
    json_object_put(root); // free
    return false;
  }
  if (!spam) {
    Logger::debug("Block::onlineCheck: '%s' is no spam", rNumber.c_str());
    json_object_put(root); // free
    return false;
  }
  Logger::debug("Block::onlineCheck: '%s' is spam", rNumber.c_str());

  *pListName = pSettings->onlineCheck;
  (void)Utils::getObject(root, "name", false, "script result", pCallerName, "");
  int score;
  if (Utils::getObject(root, "score", false, "script result", &score, 0)) {
    *pScore = std::to_string(score);
  }
  json_object_put(root); // free

  if (pSettings->onlineCache) {
    Logger::debug("Block::onlineCheck: '%s' is spam -> add to cache", rNumber.c_str());
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

bool Block::executeScript(std::string prefix, std::string scriptBaseName, const std::string& rNumber, const bool validNumber,
                          struct json_object** pRoot) {
  if (scriptBaseName.length() == 0) {
    // not enabled
    return false;
  }

  Logger::debug("Block::executeScript(prefix='%s' ,scriptBaseName='%s', rNumber='%s', validNumber=%d)",
    prefix.c_str(), scriptBaseName.c_str(), rNumber.c_str(), validNumber);

  if (Utils::startsWith(rNumber, "**")) {
    Logger::debug("Block::executeScript: is an intern number, thus makes no sense to ask the world");
    return false;
  }
  if (!validNumber) {
    Logger::debug("Block::executeScript: number is invalid, thus makes no sense to ask the world");
    return false;
  }

  std::string script = DATADIR "/callblocker/" + prefix + scriptBaseName + ".py";
  std::string parameters = "--number " + rNumber;
  // optional credential parameters  
  std::vector<struct SettingOnlineCredential> creds = m_pSettings->getOnlineCredentials();
  for (size_t i = 0; i < creds.size(); i++) {
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

  return Utils::parseJson(res, pRoot);
}
