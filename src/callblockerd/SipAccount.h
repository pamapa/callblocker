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

#ifndef SIPACCOUNT_H
#define SIPACCOUNT_H

#include <pjsua-lib/pjsua.h>

#include "SipPhone.h"
struct SettingSipAccount;


class SipAccount {
private:
  SipPhone* m_phone;
  pjsua_acc_id m_accId;

public:
  SipAccount(SipPhone* phone);
  virtual ~SipAccount();
  bool add(struct SettingSipAccount* acc);

  // callback -> class method call conversion
  static void onIncomingCallCB(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata);

private:
  void onIncomingCall(pjsua_call_id call_id, pjsip_rx_data *rdata);
  bool parseURI(pj_str_t* uri_str, std::string* display, std::string* user);
};

#endif

