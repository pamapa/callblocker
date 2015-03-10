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

#ifndef BLOCK_H
#define BLOCK_H

#include <string>

#include "FileLists.h"
#include "Settings.h"


class Block {
private:
  FileLists* m_pWhitelists;
  FileLists* m_pBlacklists;

public:
  Block();
  virtual ~Block();
  void run();
  bool isNumberBlocked(const struct SettingBase* pSettings, const std::string& rNumber, std::string* pMsg);

private:
  bool isWhiteListed(const struct SettingBase* pSettings, const std::string& rNumber, std::string* pMsg);
  bool isBlacklisted(const struct SettingBase* pSettings, const std::string& rNumber, std::string* pMsg);
};

#endif

