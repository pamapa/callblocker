/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2016 Patrick Ammann <pammann@gmx.net>

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

#include "Test.h"

#include <stdio.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include "Settings.h"
#include "Block.h"


static std::string getModuleFileName() {
  char path[PATH_MAX];
  char dest[PATH_MAX];
  pid_t pid = getpid();
  sprintf(path, "/proc/%d/exe", pid);
  assert(readlink(path, dest, PATH_MAX) != -1);
  std::string ret = dest;
  return ret;
}

static std::string getTestEtcPath() {
  std::string etc = getModuleFileName();
  std::string::size_type pos = etc.find_last_of("/\\");
  assert(pos != std::string::npos);
  etc.erase(pos);
  etc += "/data/etc";
  return etc;
}

static void TestCase_block()
{
  std::string etc = getTestEtcPath();
  //printf("etc: %s\n", etc.c_str());
  Settings* settings = new Settings(etc);
  Block* block = new Block(settings);

  SettingBase settingsBase;
  settingsBase.blockMode = WHITELISTS_AND_BLACKLISTS;
  // CH
  settingsBase.countryCode = "+41";
  std::string msg;
  // test single number in blacklist
  assert(block->isBlocked(&settingsBase, "0449999999", &msg) == true);
  assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41449999999", &msg) == true);
  assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blacklist='main'") == 0);

  // test range in blacklist
  assert(block->isBlocked(&settingsBase, "0448881111", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881111' name='Test range' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41448881111", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881111' name='Test range' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "0448881234", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881234' name='Test range' blocked blacklist='main'") == 0);

  //printf("msg: %s\n", msg.c_str());
}

void Test_Block_Run()
{
  printf("Test_Block_Run...\n");
  
  TestCase_block();
}

