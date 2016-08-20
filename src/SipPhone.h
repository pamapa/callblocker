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

#ifndef SIPPHONE_H
#define SIPPHONE_H

#if 0
#include <pjsua-lib/pjsua.h>
#endif

#include "Phone.h"


class SipPhone : public Phone {
private:
#if 0
  pj_pool_t* m_Pool;
  pjmedia_port* m_mediaPortSilence;
  pjsua_conf_port_id m_mediaConfSilenceId;
#endif

public:
  SipPhone(Block* pBlock);
  virtual ~SipPhone();
  bool init();

#if 0
  pjsua_conf_port_id getMediaConfSilenceId() { return m_mediaConfSilenceId; }
#endif
private:
  bool init_pjsua();
  bool init_pjmedia();
};

#endif

