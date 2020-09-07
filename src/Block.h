/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2019 Patrick Ammann <pammann@gmx.net>

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

#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include <json-c/json.h>

#include "FileListsNotified.h"
#include "FileListsCache.h"
#include "Settings.h"


#define BLOCK_ANONYMOUS_NUMBER_STR "anonymous"


class Block {
private:
  Settings* m_pSettings;
  FileListsNotified* m_pAllowlists;
  FileListsNotified* m_pBlocklists;
  FileListsCache* m_pCache;

public:
  Block(Settings* pSettings);
  virtual ~Block();
  void run();
  bool isBlocked(const struct SettingBase* pSettings, const std::string& rNumber, const std::string& rName, std::string* pMsg);

private:
  bool isAnonymousNumberBlocked(const struct SettingBase* pSettings, std::string* pMsg);
  bool isNumberBlocked(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber, const std::string& rName, std::string* pMsg);

  bool isAllowListed(const struct SettingBase* pSettings, const std::string& rNumber, std::string* pListName, std::string* pName);
  bool isBlocklisted(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber,
                     std::string* pListName, std::string* pName, std::string* pScore);

  void onlineLookup(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber,
                    std::string* pCallerName);
  bool onlineCheck(const struct SettingBase* pSettings, const std::string& rNumber, const bool validNumber,
                   std::string* pListName, std::string* pCallerName, std::string* pScore);
  bool executeScript(std::string prefix, std::string name, const std::string& rNumber, const bool validNumber,
                     struct json_object** root);
};

#endif

