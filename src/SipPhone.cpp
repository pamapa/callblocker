/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2017 Patrick Ammann <pammann@gmx.net>

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
#include "Utils.h"

// pjsua_init fails otherwise with the second time
// transport_srtp  ..Failed to initialize libsrtp: unsupported parameter
//  pjsua_media.c  ..Error initializing SRTP library: unsupported parameter [status=259801]
static bool s_Initialized = false;


SipPhone::SipPhone(Block* pBlock) : Phone(pBlock) {
  Logger::debug("SipPhone::SipPhone()...");
#if 0
  m_mediaPortSilence = NULL;
  m_mediaConfSilenceId = -1;
#endif
}

SipPhone::~SipPhone() {
  Logger::debug("SipPhone::~SipPhone()...");

  pjsua_call_hangup_all();

#if 0
  pjsua_conf_remove_port(m_mediaConfSilenceId);
  pjmedia_port_destroy(m_mediaPortSilence);

  pj_pool_release(m_Pool);
#endif

#if 0 // see s_Initialized
  pjsua_destroy();
#endif
}

bool SipPhone::init() {
  Logger::debug("SipPhone::init...");

  if (!s_Initialized)
  {
    if (!init_pjsua()) return false;
    if (!init_pjmedia()) return false;
    s_Initialized = true;
  }

  return true;
}

bool SipPhone::init_pjsua() {
  Logger::debug("SipPhone::init_pjsua...");

  // create pjsua  
  pj_status_t status = pjsua_create();
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_create() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return false;
  }

  // configure pjsua
  pjsua_config ua_cfg;
  pjsua_config_default(&ua_cfg);
  // enable just 1 simultaneous call 
  ua_cfg.max_calls = 1; // TODO
  // callback configuration
  ua_cfg.cb.on_reg_state2 = &SipAccount::onRegState2CB;
  ua_cfg.cb.on_call_state = &SipAccount::onCallStateCB;
  ua_cfg.cb.on_incoming_call = &SipAccount::onIncomingCallCB;
  //ua_cfg.cb.on_call_media_state = &SipAccount::onCallMediaStateCB;

  // logging configuration
  pjsua_logging_config log_cfg;    
  pjsua_logging_config_default(&log_cfg);
  log_cfg.level = pj_log_get_level();

  // media configuration
  pjsua_media_config media_cfg;
  pjsua_media_config_default(&media_cfg);
  media_cfg.clock_rate = 8000; // TODO: default of 16000 seems not to work :-(

  // initialize pjsua 
  status = pjsua_init(&ua_cfg, &log_cfg, &media_cfg);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_init() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return false;
  }

  // add udp transport
  pjsua_transport_config udpcfg;
  pjsua_transport_config_default(&udpcfg);

  status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &udpcfg, NULL);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_transport_create() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return false;
  }

  // disable sound device - use null sound device
  status = pjsua_set_null_snd_dev();
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_set_null_snd_dev() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return false;
  }

  // initialization is done, start pjsua
  status = pjsua_start();
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_start() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return false;
  }

#if 0
  m_Pool = pjsua_pool_create("SipPhone.cpp", 128, 128);
  if (m_Pool == NULL) {
    Logger::error("pjsua_pool_create() failed");
    return false;
  }
#endif

  return true;
}

bool SipPhone::init_pjmedia() {
#if 0
  Logger::debug("SipPhone::init_pjmedia...");
#define CLOCK_RATE        8000
#define CHANNEL_COUNT     1
#define SAMPLES_PER_FRAME ((CLOCK_RATE * CHANNEL_COUNT * PJSUA_DEFAULT_AUDIO_FRAME_PTIME) / 1000)

  pj_status_t status = pjmedia_null_port_create(m_Pool, CLOCK_RATE, 1, SAMPLES_PER_FRAME, 16, &m_mediaPortSilence);
  if (status != PJ_SUCCESS) {
    Logger::error("pjmedia_null_port_create() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return false;
  }

  status = pjsua_conf_add_port(m_Pool, m_mediaPortSilence, &m_mediaConfSilenceId);
  if (status != PJ_SUCCESS) {
    Logger::error("pjsua_conf_add_port() failed (%s)", Utils::getPjStatusAsString(status).c_str());
    return false;
  }
#endif
  return true;
}

