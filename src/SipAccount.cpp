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

#include "SipAccount.h" // API

#include <string>
#include <sstream>
#include <pjsua-lib/pjsua.h>

#include "Logger.h"
#include "Settings.h"


static std::string getStatusAsString(pj_status_t status) {
  static char buf[100];
  pj_str_t pjstr = pj_strerror(status, buf, sizeof(buf)); 
  std::string ret = pj_strbuf(&pjstr);
  return ret;
}

SipAccount::SipAccount(SipPhone* phone) {
  m_phone = phone;
  m_accId = -1;
}

SipAccount::~SipAccount() {
  Logger::debug("~SipAccount...");
  m_phone = NULL;

  if (m_accId == -1) {
    return;
  }

  (void)pjsua_acc_set_user_data(m_accId, NULL);

  pj_status_t status = pjsua_acc_del(m_accId);
  m_accId = -1;
  if (status != PJ_SUCCESS) {
    Logger::warn("pjsua_acc_del() failed (%s)", getStatusAsString(status).c_str());
  }
}

bool SipAccount::add(struct SettingSipAccount* acc) {
  Logger::debug("SipAccount::add(%s)...", acc->toString().c_str());
  m_settings = *acc; // strcut copy

  // prepare account configuration
  pjsua_acc_config cfg;
  pjsua_acc_config_default(&cfg);
  
  std::ostringstream user_url_ss;
  user_url_ss << "sip:" << acc->fromUsername << "@" << acc->fromDomain;
  std::string user_url = user_url_ss.str();
  
  std::ostringstream provider_url_ss;
  provider_url_ss << "sip:" << acc->fromDomain;
  std::string provider_url = provider_url_ss.str();

  // create and define account
  cfg.id = pj_str((char*)user_url.c_str());
  cfg.reg_uri = pj_str((char*)provider_url.c_str());
  cfg.cred_count = 1;
  cfg.cred_info[0].realm = pj_str((char*)acc->fromDomain.c_str());
  cfg.cred_info[0].scheme = pj_str((char*)"digest");
  cfg.cred_info[0].username = pj_str((char*)acc->fromUsername.c_str());
  cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
  cfg.cred_info[0].data = pj_str((char*)acc->fromPassword.c_str());

  // add account
  pj_status_t status = pjsua_acc_add(&cfg, PJ_TRUE, &m_accId);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_acc_add() failed (%s)", getStatusAsString(status).c_str());
    return false;
  }

  status = pjsua_acc_set_user_data(m_accId, this);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_acc_set_user_data() failed (%s)", getStatusAsString(status).c_str());
    return false;
  }
}

void SipAccount::onIncomingCallCB(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata) {
  SipAccount* p = (SipAccount*)pjsua_acc_get_user_data(acc_id);
  p->onIncomingCall(call_id, rdata);
}

void SipAccount::onIncomingCall(pjsua_call_id call_id, pjsip_rx_data *rdata)
{
  Logger::debug("SipAccount::onIncomingCall...");

  PJ_UNUSED_ARG(rdata);

  pjsua_call_info ci;
  pjsua_call_get_info(call_id, &ci);

  Logger::debug("local_info %s", pj_strbuf(&ci.local_info));
  Logger::debug("local_contact %s", pj_strbuf(&ci.local_contact));
  Logger::debug("remote_info %s", pj_strbuf(&ci.remote_info));
  Logger::debug("remote_contact %s", pj_strbuf(&ci.remote_contact));
  Logger::debug("call_id %s", pj_strbuf(&ci.call_id));

  std::string display, user;
  if (!parseURI(&ci.remote_info, &display, &user)) {
    Logger::warn("invalid URI received '%s'", pj_strbuf(&ci.remote_info));
    return;
  }

  std::string reason;
  bool block = m_phone->isNumberBlocked(m_settings.blockMode, user, &reason);

  std::string msg;
  msg += "Incoming call from ";
  msg += user;
  if (block) {
    msg += " is blocked";
  }
  if (reason.length() > 0) {
    msg += " (";
    msg += reason;
    msg += ")";
  }
  Logger::notice(msg.c_str());

  // TODO: 302 redirect ??
#if 0
  Use pjsua_call_hangup() and put the destination URL in the Contact
  header of the pjsua_msg_data.

  pj_pool_t* pool = pjsua_pool_create("", 512, 512);
  pjsua_msg_data msgData;
  pjsua_msg_data_init(&msgData);

  pj_str_t tmp;
  pjsip_generic_string_hdr* hdr =
    pjsip_generic_string_hdr_create(pool, pj_cstr(&tmp, "Contact"), pj_cstr(&tmp, "URI ...TODO"));
  pj_list_push_back(&msgData.hdr_list, hdr);
  pjsua_call_hangup(call_id, 302, NULL, &msgData);
  pj_pool_release(pool);
#endif

  if (block) {
    // codes: http://de.wikipedia.org/wiki/SIP-Status-Codes 603=Declined
    pj_status_t status = pjsua_call_hangup(call_id, 603, NULL, NULL);
    if (status != PJ_SUCCESS) {
      Logger::warn("pjsua_call_hangup() failed (%s)", getStatusAsString(status).c_str());
    }
  }
}

bool SipAccount::parseURI(pj_str_t* uri, std::string* display, std::string* user) {
  pj_pool_t* pool = pjsua_pool_create("", 128, 10);
  pjsip_name_addr* n = (pjsip_name_addr*)pjsip_parse_uri(pool, uri->ptr, uri->slen, PJSIP_PARSE_URI_AS_NAMEADDR);
  if (n == NULL) {
    Logger::warn("pjsip_parse_uri() failed for %s", pj_strbuf(uri));
    pj_pool_release(pool);
    return false;
  }
  if (!PJSIP_URI_SCHEME_IS_SIP(n)) {
    Logger::warn("pjsip_parse_uri() returned unknown schema for %s", pj_strbuf(uri));
    pj_pool_release(pool);
    return false;
  }

  *display = std::string(n->display.ptr, n->display.slen);

  pjsip_sip_uri *sip = (pjsip_sip_uri*)pjsip_uri_get_uri(n);
  *user = std::string(sip->user.ptr, sip->user.slen);

  pj_pool_release(pool);
  return true;
}

