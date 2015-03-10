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

#ifndef ANALOGPHONE_H
#define ANALOGPHONE_H

#include <string>
#include <termios.h>

#include "Phone.h"


class AnalogPhone : public Phone {
private:
  struct SettingAnalogPhone m_settings;

  int m_FD;
  struct termios m_origTermios;

  time_t m_ringTime;
  unsigned int m_numRings;
  bool m_foundCID;

public:
  AnalogPhone(Block* pBlock);
  virtual ~AnalogPhone();
  bool init(struct SettingAnalogPhone* pPhone);
  void run();

private:
  bool openDevice();
  bool sendCommand(std::string cmd);
  bool getData(std::string* data);
};

#endif

