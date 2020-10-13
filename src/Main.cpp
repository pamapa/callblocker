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

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include "Logger.h"
#include "Settings.h"
#include "SipPhone.h"
#include "SipAccount.h"
#include "AnalogPhone.h"


#define LOOP_WAIT_TIME_USEC    (500 * 1000)  // 500 milliseconds


static bool s_appRunning = true;
static bool s_appReloadConfig = false;

static void signal_handler(int signal) {
  if (signal == SIGHUP) {
    s_appReloadConfig = true;
    return;
  }

  Logger::info("exiting (signal %i received)...", signal);
  s_appRunning = false;
}


class Main {
private:
  Settings* m_pSettings;
  Block* m_pBlock;
  SipPhone* m_pSipPhone;
  std::vector<SipAccount*> m_sipAccounts;
  std::vector<AnalogPhone*> m_analogPhones;

public:
  Main() {
    Logger::start(true);

    m_pSettings = new Settings(SYSCONFDIR "/" PACKAGE_NAME);
    m_pBlock = new Block(m_pSettings);

    m_pSipPhone = nullptr;
    add();
  }

  virtual ~Main() {
    remove();
    delete m_pBlock;
    delete m_pSettings;
    Logger::stop();
  }

  void loop() {
    Logger::debug("Main::loop() enter main loop...");
    while (s_appRunning) {
      m_pBlock->run();

      if (s_appReloadConfig || m_pSettings->hasChanged()) {
        Logger::info("reload phones");
        remove();
        add();
        s_appReloadConfig = false;
      }

      for (const auto& analogPhone : m_analogPhones) {
        analogPhone->run();
      }

      (void)usleep(LOOP_WAIT_TIME_USEC);
    }
  }

private:
  void remove() {
    // Analog
    for (const auto& analogPhone : m_analogPhones) {
      delete analogPhone;
    }
    m_analogPhones.clear();

    // SIP
    for (const auto& sipAccount : m_sipAccounts) {
      delete sipAccount;
    }
    m_sipAccounts.clear();
    if (m_pSipPhone != nullptr) {
      delete m_pSipPhone;
      m_pSipPhone = nullptr;
    }
  }

  void add() {
    // Analog
    std::vector<struct SettingAnalogPhone> analogPhones = m_pSettings->getAnalogPhones();
    for (const auto& analogPhone : analogPhones) {
      AnalogPhone* tmp = new AnalogPhone(m_pBlock);
      if (tmp->init(&analogPhone)) m_analogPhones.push_back(tmp);
      else delete tmp;
    }

    // SIP
    std::vector<struct SettingSipAccount> accounts = m_pSettings->getSipAccounts();
    for (const auto& account : accounts) {
      if (m_pSipPhone == nullptr) {
        m_pSipPhone = new SipPhone(m_pBlock);
        if (!m_pSipPhone->init()) {
          break;
        }
      }
      SipAccount* tmp = new SipAccount(m_pSipPhone);
      if (tmp->add(&account)) m_sipAccounts.push_back(tmp);
      else delete tmp;
    }
  }
};


int main(int argc, char *argv[]) {
  // register signal handler for break-in-keys (e.g. ctrl+c)
  signal(SIGINT, signal_handler);
  signal(SIGKILL, signal_handler);
  // register signal handler for systemd shutdown request
  signal(SIGTERM, signal_handler);
  // register signal handler for systemd reload the configuration files request
  signal(SIGHUP, signal_handler);

  Logger::info("starting callblockerd %s", VERSION);

  Main* m = new Main();
  m->loop();
  delete m;

  return 0;
}

