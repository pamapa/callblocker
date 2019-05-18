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

#include "SipAccount.h" // API

#include <string>
#include <sstream>
#include <unistd.h>
#include <sys/socket.h>

#include <pjsua-lib/pjsua.h>

#include "Logger.h"
#include "Settings.h"
#include "Utils.h"


SipAccount::SipAccount(SipPhone* pPhone) {
  Logger::debug("SipAccount::SipAccount()");
  m_pPhone = pPhone;
  m_accId = -1;
}

SipAccount::~SipAccount() {
  Logger::debug("SipAccount::~SipAccount()");
  m_pPhone = NULL;

  if (m_accId == -1) {
    return;
  }

  (void)pjsua_acc_set_user_data(m_accId, NULL);

  pj_status_t status = pjsua_acc_del(m_accId);
  m_accId = -1;
  if (status != PJ_SUCCESS) {
    Logger::warn("pjsua_acc_del() failed (%s)", Utils::getPjStatusAsString(status).c_str());
  }
}

bool SipAccount::add(struct SettingSipAccount* pSettings) {
  Logger::debug("SipAccount::add(%s)", Settings::toString(pSettings).c_str());
  m_settings = *pSettings; // struct copy

  // prepare account configuration
  pjsua_acc_config cfg;
  pjsua_acc_config_default(&cfg);

  std::string schema = "sip:";
  if (m_settings.secure) {
    schema = "sips:";
  }

  // login id
  std::string id = schema + m_settings.username + "@" + m_settings.domain;

  // connection uri
  std::string reg_uri = schema + m_settings.domain;
  if (m_settings.forceIPv4) {
    std::string domain_ipv4;
    if (!Utils::resolveHostname(m_settings.domain, AF_INET, &domain_ipv4)) {
      Logger::error("resolving hostname %s to IPv4 address failed", m_settings.domain.c_str());
      return false;
    }
    reg_uri = schema + domain_ipv4;
  }

  // create and define account
  cfg.id = pj_str((char*)id.c_str());
  cfg.reg_uri = pj_str((char*)reg_uri.c_str());
  cfg.cred_count = 1;
  cfg.cred_info[0].realm = pj_str((char*)m_settings.realm.c_str());
  cfg.cred_info[0].scheme = pj_str((char*)"digest");
  cfg.cred_info[0].username = pj_str((char*)m_settings.username.c_str());
  cfg.cred_info[0].data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
  cfg.cred_info[0].data = pj_str((char*)m_settings.password.c_str());

  // outbound proxy
  if (!m_settings.outboundProxy.empty()) {
    cfg.reg_use_proxy = 3;
    cfg.proxy_cnt = 1;
    cfg.proxy[0] = pj_str((char*)m_settings.outboundProxy.c_str());
  }

  // add account
  pj_status_t status = pjsua_acc_add(&cfg, PJ_TRUE, &m_accId);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_acc_add() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return false;
  }

  status = pjsua_acc_set_user_data(m_accId, this);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_acc_set_user_data() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return false;
  }

  return true;
}

void SipAccount::onRegState2CB(pjsua_acc_id acc_id, pjsua_reg_info *info) {
  SipAccount* p = (SipAccount*)pjsua_acc_get_user_data(acc_id);
  if (p == NULL) {
    Logger::warn("onRegState2CB(acc_id=%d, ...) account not found", acc_id);
    return;
  }
  p->onRegState2(info);
}

void SipAccount::onRegState2(pjsua_reg_info *info) {
  if (info->cbparam->code == 200) {
    Logger::debug("SipAccount::onRegState2(): successfully %s on domain '%s' with username '%s'", info->renew ? "registered" : "unregistered",
                 m_settings.domain.c_str(), m_settings.username.c_str());
  } else {
    Logger::warn("%s on domain '%s' with username '%s' failed (code=%d)", info->renew ? "registration" : "unregistration",
                 m_settings.domain.c_str(), m_settings.username.c_str(), info->cbparam->code);
  }
}

void SipAccount::onIncomingCallCB(pjsua_acc_id acc_id, pjsua_call_id call_id, pjsip_rx_data *rdata) {
  SipAccount* p = (SipAccount*)pjsua_acc_get_user_data(acc_id);
  if (p == NULL) {
    Logger::warn("onIncomingCallCB(acc_id=%d, call_id=%d) account not found", acc_id, call_id);
    return;
  }
  p->onIncomingCall(call_id, rdata);
}

void SipAccount::onIncomingCall(pjsua_call_id call_id, pjsip_rx_data *rdata) {
  Logger::debug("SipAccount::onIncomingCall(call_id=%d)...", call_id);
  PJ_UNUSED_ARG(rdata);

  pj_status_t status = pjsua_call_set_user_data(call_id, this);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_acc_set_user_data() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return;
  }

  pjsua_call_info ci;
  pjsua_call_get_info(call_id, &ci);

#if 0
  Logger::debug("local_info %s", pj_strbuf(&ci.local_info));
  Logger::debug("local_contact %s", pj_strbuf(&ci.local_contact));
  Logger::debug("remote_info %s", pj_strbuf(&ci.remote_info));
  Logger::debug("remote_contact %s", pj_strbuf(&ci.remote_contact));
  Logger::debug("call_id %s", pj_strbuf(&ci.call_id));
#endif

  std::string display, number;
  if (!getNumber(&ci.remote_info, &display, &number)) {
    Logger::warn("invalid URI received '%s'", pj_strbuf(&ci.remote_info));
    return;
  }

  std::string msg;
  bool block = m_pPhone->isBlocked(&m_settings.base, number, "", &msg);
  Logger::notice(msg.c_str());

#if 0
  // 302 redirect
  Use pjsua_call_hangup() and put the destination URL in the Contact
  header of the pjsua_msg_data.

  pj_pool_t* pool = pjsua_pool_create("", 512, 512);
  pjsua_msg_data msgData;
  pjsua_msg_data_init(&msgData);

  pj_str_t tmp;
  pjsip_generic_string_hdr* hdr =
    pjsip_generic_string_hdr_create(pool, pj_cstr(&tmp, "Contact"), pj_cstr(&tmp, "URI ...TODO"));
  pj_list_push_back(&msgData.hdr_list, hdr);
  // codes: http://de.wikipedia.org/wiki/SIP-Status-Codes
   // enum pjsip_status_code...
  pjsua_call_hangup(call_id, PJSIP_SC_MOVED_TEMPORARILY, NULL, &msgData);
  pj_pool_release(pool);
#endif

  if (block) {
    // answer incoming calls with 200/OK, then we hangup in onCallState...
    pj_status_t status = pjsua_call_answer(call_id, 200, NULL, NULL);
    if (status != PJ_SUCCESS) {
      Logger::warn("pjsua_call_answer() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    }
  }
}

void SipAccount::onCallStateCB(pjsua_call_id call_id, pjsip_event* e) {
  SipAccount* p = (SipAccount*)pjsua_call_get_user_data(call_id);
  if (p == NULL) {
    Logger::warn("onCallStateCB(call_id=%d) account not found", call_id);
    return;
  }
  p->onCallState(call_id, e);
}

void SipAccount::onCallState(pjsua_call_id call_id, pjsip_event* e) {
  Logger::debug("SipAccount::onCallState(call_id=%d)...", call_id);
  PJ_UNUSED_ARG(e);

  pjsua_call_info ci;
  pjsua_call_get_info(call_id, &ci);

  // NOTE: if the number is not blocked, we would not be here
  std::string state = std::string(pj_strbuf(&ci.state_text), ci.state_text.slen);
  Logger::debug("SipAccount::onCallState(): call state changed to %s", state.c_str());

  if (ci.state == PJSIP_INV_STATE_CONFIRMED) {
    Logger::debug("SipAccount::onCallState(): hangup...");
    // code 0: pj takes care of hangup SIP status code
    pj_status_t status = pjsua_call_hangup(call_id, 0, NULL, NULL);
    if (status != PJ_SUCCESS) {
      Logger::warn("pjsua_call_hangup() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    }
  }
}

#if 0
void SipAccount::onCallMediaStateCB(pjsua_call_id call_id) {
  SipAccount* p = (SipAccount*)pjsua_call_get_user_data(call_id);
  if (p == NULL) {
    Logger::warn("onCallMediaStateCB(%d) failed", call_id);
    return;
  }
  p->onCallMediaState(call_id);
}

void SipAccount::onCallMediaState(pjsua_call_id call_id) {
  Logger::debug("SipAccount::onCallMediaState(call_id=%d)...", call_id);

  pjsua_call_info ci;
  pjsua_call_get_info(call_id, &ci);

  if (ci.media_status == PJSUA_CALL_MEDIA_ACTIVE) {
    // connect active call to silence
    pjsua_conf_connect(m_pPhone->getMediaConfSilenceId(), ci.conf_slot);
  }
}
#endif

bool SipAccount::getNumber(pj_str_t* uri, std::string* pDisplay, std::string* pNumber) {
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

  *pDisplay = std::string(n->display.ptr, n->display.slen);

  pjsip_sip_uri *sip = (pjsip_sip_uri*)pjsip_uri_get_uri(n);
  *pNumber = std::string(sip->user.ptr, sip->user.slen);

  pj_pool_release(pool);
  return true;
}

