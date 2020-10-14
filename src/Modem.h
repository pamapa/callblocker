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

#ifndef MODEM_H
#define MODEM_H

#include <string>
#include <termios.h>

class Modem {
private:
    std::string m_name;
    int m_FD;
    struct termios m_origTermios;

public:
    Modem();
    virtual ~Modem();

    bool open(std::string name);
    bool sendCommand(std::string cmd);
    bool getData(std::string* data);
};

#endif
