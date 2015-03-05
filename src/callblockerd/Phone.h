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

#ifndef PHONE_H
#define PHONE_H

#include "Lists.h"


class Phone {
private:
  Lists* m_whitelists;
  Lists* m_blacklists;

public:
  Phone(Lists* whitelists, Lists* blacklists) {
    m_whitelists = whitelists;
    m_blacklists = blacklists;
  }
  virtual ~Phone() {
    m_whitelists = NULL;
    m_blacklists = NULL;
  }
  virtual bool init() = 0;

  bool isWhitelisted(const char* number) { return m_whitelists->isListed(number); }
  bool isBlacklisted(const char* number) { return m_blacklists->isListed(number); }
};

#endif

