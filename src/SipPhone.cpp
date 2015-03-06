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

#include "SipPhone.h" // API

#include <string>
#include <pjsua-lib/pjsua.h>

#include "Logger.h"
#include "SipAccount.h"


// TODO
// -reconnect on failure
// -reconnect once a day?
// PHP:
// ps -A | grep systemd-journal
//  213 ?        00:00:00 systemd-journal
// journalctl <executable full path> --output=short --lines=100


static std::string getStatusAsString(pj_status_t status) {
  static char buf[100];
  pj_str_t pjstr = pj_strerror(status, buf, sizeof(buf)); 
  std::string ret = pj_strbuf(&pjstr);
  return ret;
}

SipPhone::SipPhone(Lists* whitelists, Lists* blacklists) : Phone(whitelists, blacklists) {
}

SipPhone::~SipPhone() {
  Logger::debug("~SipPhone...");
  // hangup open calls and stop pjsua
  pjsua_call_hangup_all();
  pjsua_destroy();
}

bool SipPhone::init() {
  Logger::debug("SipPhone::init...");

  // create pjsua  
  pj_status_t status = pjsua_create();
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_create() failed (%s)", getStatusAsString(status).c_str());
    return false;
  }

  // configure pjsua
  pjsua_config ua_cfg;
  pjsua_config_default(&ua_cfg);
  // enable just 1 simultaneous call 
  //ua_cfg.max_calls = 1;
  // callback configuration
  ua_cfg.cb.on_call_state = &SipAccount::onCallStateCB;
  ua_cfg.cb.on_incoming_call = &SipAccount::onIncomingCallCB;

  // logging configuration
  pjsua_logging_config log_cfg;    
  pjsua_logging_config_default(&log_cfg);
  log_cfg.level = pj_log_get_level();
  
  // media configuration
  pjsua_media_config media_cfg;
  pjsua_media_config_default(&media_cfg);

  // initialize pjsua 
  status = pjsua_init(&ua_cfg, &log_cfg, &media_cfg);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_init() failed (%s)", getStatusAsString(status).c_str());
    return false;
  }
  
  // add udp transport
  pjsua_transport_config udpcfg;
  pjsua_transport_config_default(&udpcfg);
  //udpcfg.port = 5060;

  status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &udpcfg, NULL);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_transport_create() failed (%s)", getStatusAsString(status).c_str());
    return false;
  }

  // disable sound - use null sound device
  status = pjsua_set_null_snd_dev();
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_set_null_snd_dev() failed (%s)", getStatusAsString(status).c_str());
    return false;
  }

  // initialization is done, start pjsua
  status = pjsua_start();
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_start() failed (%s)", getStatusAsString(status).c_str());
    return false;
  }

  return true;
}

