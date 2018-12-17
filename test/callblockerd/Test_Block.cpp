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
#include <string.h>
#include <linux/limits.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include "Settings.h"
#include "Utils.h"
#include "Block.h"


static void TestCase_logging_only(std::string exePath)
{
  std::string etc = Utils::pathJoin(exePath, "data/etc");
  //printf("etc: %s\n", etc.c_str());
  Settings* settings = new Settings(etc);
  Block* block = new Block(settings);

  SettingBase settingsBase;
  settingsBase.blockAnonymousCID = false;
  settingsBase.blockInvalidCID = false;
  settingsBase.blockMode = LOGGING_ONLY;
  // CH
  settingsBase.countryCode = "+41";
  std::string msg;
  // test number in blacklist
  assert(block->isBlocked(&settingsBase, "0449999999", "", &msg) == false);
  //printf("msg: %s\n", msg.c_str());
  assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41449999999", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blacklist='main'") == 0);

  // test range in blacklist
  assert(block->isBlocked(&settingsBase, "0448881111", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41448881111", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "0448881234", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41448881234' name='Test range 1' blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+23412312312", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+23412312312' name='Test range 2' blacklist='main'") == 0);

  // test number in whitelist
  assert(block->isBlocked(&settingsBase, "0441112233", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441112233' name='Mr. Whitelist 1' whitelist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41441112233", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441112233' name='Mr. Whitelist 1' whitelist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+4144222", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+4144222' name='Mr. Whitelist 3 too small' invalid whitelist='main'") == 0);

  // test number in whitelist and blacklist
  assert(block->isBlocked(&settingsBase, "+41441119999", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441119999' name='Mr. Whitelist 2 in blacklist too' whitelist='main'") == 0);

  // test anonymous
  settingsBase.blockAnonymousCID = false;
  assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == false);
  assert(msg.compare("Incoming call: number='anonymous'") == 0);
  settingsBase.blockAnonymousCID = true;
  assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == false);
  assert(msg.compare("Incoming call: number='anonymous'") == 0);

  // test invalid: too small number
  settingsBase.blockInvalidCID = false;
  assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+4144333' invalid") == 0);
  settingsBase.blockInvalidCID = true;
  assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+4144333' invalid") == 0);

  // test invalid: too long number
  settingsBase.blockInvalidCID = false;
  assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+493456789012345678' invalid") == 0);
  settingsBase.blockInvalidCID = true;
  assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+493456789012345678' invalid") == 0);
}

static void TestCase_whitelists_only(std::string exePath)
{
  std::string etc = Utils::pathJoin(exePath, "data/etc");
  //printf("etc: %s\n", etc.c_str());
  Settings* settings = new Settings(etc);
  Block* block = new Block(settings);

  SettingBase settingsBase;
  settingsBase.blockAnonymousCID = false;
  settingsBase.blockInvalidCID = false;
  settingsBase.blockMode = WHITELISTS_ONLY;
  // CH
  settingsBase.countryCode = "+41";
  std::string msg;
  // test number in blacklist
  assert(block->isBlocked(&settingsBase, "0449999999", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41449999999' blocked") == 0);
  assert(block->isBlocked(&settingsBase, "+41449999999", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41449999999' blocked") == 0);

  // test range in blacklist
  assert(block->isBlocked(&settingsBase, "0448881111", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881111' blocked") == 0);
  assert(block->isBlocked(&settingsBase, "+41448881111", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881111' blocked") == 0);
  assert(block->isBlocked(&settingsBase, "0448881234", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881234' blocked") == 0);
  assert(block->isBlocked(&settingsBase, "+23412312312", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+23412312312' blocked") == 0);

  // test number in whitelist
  assert(block->isBlocked(&settingsBase, "0441112233", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441112233' name='Mr. Whitelist 1' whitelist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41441112233", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441112233' name='Mr. Whitelist 1' whitelist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+4144222", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+4144222' name='Mr. Whitelist 3 too small' invalid whitelist='main'") == 0);

  // test number in whitelist and blacklist
  assert(block->isBlocked(&settingsBase, "+41441119999", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441119999' name='Mr. Whitelist 2 in blacklist too' whitelist='main'") == 0);

  // test anonymous
  settingsBase.blockAnonymousCID = false;
  assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == true);
  assert(msg.compare("Incoming call: number='anonymous' blocked") == 0);
  settingsBase.blockAnonymousCID = true;
  assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == true);
  assert(msg.compare("Incoming call: number='anonymous' blocked") == 0);

  // test invalid: too small number
  settingsBase.blockInvalidCID = false;
  assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+4144333' blocked invalid") == 0);
  settingsBase.blockInvalidCID = true;
  assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+4144333' blocked invalid") == 0);

  // test invalid: too long number
  settingsBase.blockInvalidCID = false;
  assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+493456789012345678' blocked invalid") == 0);
  settingsBase.blockInvalidCID = true;
  assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+493456789012345678' blocked invalid") == 0);
}

static void TestCase_whitelists_and_blacklists(std::string exePath)
{
  std::string etc = Utils::pathJoin(exePath, "data/etc");
  //printf("etc: %s\n", etc.c_str());
  Settings* settings = new Settings(etc);
  Block* block = new Block(settings);

  SettingBase settingsBase;
  settingsBase.blockAnonymousCID = false;
  settingsBase.blockInvalidCID = false;
  settingsBase.blockMode = WHITELISTS_AND_BLACKLISTS;
  // CH
  settingsBase.countryCode = "+41";
  std::string msg;
  // test number in blacklist
  assert(block->isBlocked(&settingsBase, "0449999999", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41449999999", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blacklist='main'") == 0);

  // test number in range in blacklist
  assert(block->isBlocked(&settingsBase, "0448881111", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41448881111", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "0448881234", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881234' name='Test range 1' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+23412312312", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+23412312312' name='Test range 2' blocked blacklist='main'") == 0);

  // test number in whitelist
  assert(block->isBlocked(&settingsBase, "0441112233", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441112233' name='Mr. Whitelist 1' whitelist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41441112233", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441112233' name='Mr. Whitelist 1' whitelist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+4144222", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+4144222' name='Mr. Whitelist 3 too small' invalid whitelist='main'") == 0);

  // test number in whitelist and blacklist
  assert(block->isBlocked(&settingsBase, "+41441119999", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441119999' name='Mr. Whitelist 2 in blacklist too' whitelist='main'") == 0);

  // test anonymous
  settingsBase.blockAnonymousCID = false;
  assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == false);
  assert(msg.compare("Incoming call: number='anonymous'") == 0);
  settingsBase.blockAnonymousCID = true;
  assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == true);
  assert(msg.compare("Incoming call: number='anonymous' blocked") == 0);

  // test invalid: too small number
  settingsBase.blockInvalidCID = false;
  assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+4144333' invalid") == 0);
  settingsBase.blockInvalidCID = true;
  assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+4144333' blocked invalid") == 0);

  // test invalid: too long number
  settingsBase.blockInvalidCID = false;
  assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+493456789012345678' invalid") == 0);
  settingsBase.blockInvalidCID = true;
  assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+493456789012345678' blocked invalid") == 0);
}

static void TestCase_blacklists_only(std::string exePath)
{
  std::string etc = Utils::pathJoin(exePath, "data/etc");
  //printf("etc: %s\n", etc.c_str());
  Settings* settings = new Settings(etc);
  Block* block = new Block(settings);

  SettingBase settingsBase;
  settingsBase.blockAnonymousCID = false;
  settingsBase.blockInvalidCID = false;
  settingsBase.blockMode = BLACKLISTS_ONLY;
  // CH
  settingsBase.countryCode = "+41";
  std::string msg;
  // test number in blacklist
  assert(block->isBlocked(&settingsBase, "0449999999", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41449999999", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blacklist='main'") == 0);

  // test range in blacklist
  assert(block->isBlocked(&settingsBase, "0448881111", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+41448881111", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "0448881234", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41448881234' name='Test range 1' blocked blacklist='main'") == 0);
  assert(block->isBlocked(&settingsBase, "+23412312312", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+23412312312' name='Test range 2' blocked blacklist='main'") == 0);

  // test number in whitelist
  assert(block->isBlocked(&settingsBase, "0441112233", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441112233'") == 0);
  assert(block->isBlocked(&settingsBase, "+41441112233", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+41441112233'") == 0);
  assert(block->isBlocked(&settingsBase, "+4144222", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+4144222' invalid") == 0);

  // test number in whitelist and blacklist
  assert(block->isBlocked(&settingsBase, "+41441119999", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+41441119999' name='Mr. Whitelist 2 in blacklist too' blocked blacklist='main'") == 0);

  // test anonymous
  settingsBase.blockAnonymousCID = false;
  assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == false);
  assert(msg.compare("Incoming call: number='anonymous'") == 0);
  settingsBase.blockAnonymousCID = true;
  assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == true);
  assert(msg.compare("Incoming call: number='anonymous' blocked") == 0);

  // test invalid: too short number
  settingsBase.blockInvalidCID = false;
  assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+4144333' invalid") == 0);
  settingsBase.blockInvalidCID = true;
  assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+4144333' blocked invalid") == 0);

  // test invalid: too long number
  settingsBase.blockInvalidCID = false;
  assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == false);
  assert(msg.compare("Incoming call: number='+493456789012345678' invalid") == 0);
  settingsBase.blockInvalidCID = true;
  assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == true);
  assert(msg.compare("Incoming call: number='+493456789012345678' blocked invalid") == 0);
}

void Test_Block_Run(std::string exePath)
{
  printf("Test_Block_Run...\n");
  
  TestCase_logging_only(exePath);
  TestCase_whitelists_only(exePath);
  TestCase_whitelists_and_blacklists(exePath);
  TestCase_blacklists_only(exePath);
}

