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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "Logger.h"
#include "Settings.h"
#include "Lists.h"
#include "SipPhone.h"
#include "SipAccount.h"

static bool s_appRunning = true;

static void signal_handler(int signal) {
  s_appRunning = false;
  Logger::info("exiting (signal %i received)...", signal);
}


class Handler {
private:
  Settings* m_settings;
  Lists* m_whitelists;
  Lists* m_blacklists;
  SipPhone* m_sipPhone;
  std::vector<SipAccount*> m_sipAccounts;

public:
  Handler() {
    Logger::start();
    m_settings = new Settings();

    m_whitelists = new Lists(SYSCONFDIR "/" PACKAGE_NAME "/whitelists");
    m_blacklists = new Lists(SYSCONFDIR "/" PACKAGE_NAME "/blacklists");

    m_sipPhone = new SipPhone(m_whitelists, m_blacklists);
    m_sipPhone->init();

    add();
  }

  virtual ~Handler() {
    remove();
    delete m_sipPhone;
    delete m_blacklists;
    delete m_whitelists;
    delete m_settings;
    Logger::stop();
  }

  void mainLoop() {
    while (s_appRunning) {
      m_whitelists->run();
      m_blacklists->run();

      if (m_settings->hasChanged()) {
        Logger::debug("reload SIP accounts");
        remove();
        add();
      }
    }
  }

private:
  void remove() {
    for(size_t i = 0; i < m_sipAccounts.size(); i++) {
      delete m_sipAccounts[i];
    }
    m_sipAccounts.clear();
  }

  void add() {
    std::vector<struct SettingSipAccount> accounts = m_settings->getSipAccounts();
    for(size_t i = 0; i < accounts.size(); i++) {
      SipAccount* tmp = new SipAccount(m_sipPhone);
      if (tmp->add(&accounts[i])) m_sipAccounts.push_back(tmp);
      else delete tmp;
    }
  }
};


int main(int argc, char *argv[]) {

	// register signal handler for break-in-keys (e.g. ctrl+c)
	signal(SIGINT, signal_handler);
	signal(SIGKILL, signal_handler);

  Handler* h = new Handler();
  h->mainLoop();
  delete h;

  return 0;
}

