/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2020 Patrick Ammann <pammann@gmx.net>

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

#include <assert.h>
#include <linux/limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "Block.h"
#include "Logger.h"
#include "Settings.h"
#include "Utils.h"

static void TestCase_logging_only(std::string etcPath)
{
    //printf("etc: %s\n", etcPath.c_str());
    Settings* settings = new Settings(etcPath);
    Block* block = new Block(settings);

    SettingBase settingsBase;
    settingsBase.blockAnonymousCID = false;
    settingsBase.blockInvalidCID = false;
    settingsBase.blockMode = LOGGING_ONLY;
    // CH
    settingsBase.countryCode = "+41";
    std::string msg;
    // test number in blocklist
    assert(block->isBlocked(&settingsBase, "0449999999", "", &msg) == false);
    //printf("msg: %s\n", msg.c_str());
    assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+41449999999", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocklist='main'") == 0);

    // test range in blocklist
    assert(block->isBlocked(&settingsBase, "0448881111", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+41448881111", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "0448881234", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41448881234' name='Test range 1' blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+23412312312", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+23412312312' name='Test range 2' blocklist='main'") == 0 ||
           msg.compare("Incoming call: number='+23412312312' name='Test range 2' invalid blocklist='main'") == 0);

    // test number in allowlist
    assert(block->isBlocked(&settingsBase, "0441112233", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112233' name='Mr. X 1' allowlist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+41441112233", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112233' name='Mr. X 1' allowlist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+4144222", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+4144222' name='Mr. X 3 too small' invalid allowlist='main'") == 0);

    // test number in allowlist and blocklist
    assert(block->isBlocked(&settingsBase, "+41441119999", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441119999' name='Mr. X 2 in blocklist too' allowlist='main'") == 0);

    // test: caller information from provider
    assert(block->isBlocked(&settingsBase, "+12025808292", "Mr. Spamer", &msg) == false);
    assert(msg.compare("Incoming call: number='+12025808292' name='Mr. Spamer'") == 0);

    // test anonymous number
    settingsBase.blockAnonymousCID = false;
    assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == false);
    assert(msg.compare("Incoming call: number='anonymous'") == 0);
    settingsBase.blockAnonymousCID = true;
    assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == false);
    assert(msg.compare("Incoming call: number='anonymous'") == 0);

    // test invalid number: too small number
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+4144333' invalid") == 0);
    settingsBase.blockInvalidCID = true;
    assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+4144333' invalid") == 0);

    // test invalid number: too long number
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+493456789012345678' invalid") == 0);
    settingsBase.blockInvalidCID = true;
    assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+493456789012345678' invalid") == 0);

    // test name with wildcard
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+41441112222", "SPAM: yes", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112222' name='SPAM: yes' blocklist='main'") == 0);
}

static void TestCase_allowlists_only(std::string etcPath)
{
    //printf("etc: %s\n", etcPath.c_str());
    Settings* settings = new Settings(etcPath);
    Block* block = new Block(settings);

    SettingBase settingsBase;
    settingsBase.blockAnonymousCID = false;
    settingsBase.blockInvalidCID = false;
    settingsBase.blockMode = ALLOWLISTS_ONLY;
    // CH
    settingsBase.countryCode = "+41";
    std::string msg;
    // test number in blocklist
    assert(block->isBlocked(&settingsBase, "0449999999", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41449999999' blocked") == 0);
    assert(block->isBlocked(&settingsBase, "+41449999999", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41449999999' blocked") == 0);

    // test range in blocklist
    assert(block->isBlocked(&settingsBase, "0448881111", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41448881111' blocked") == 0);
    assert(block->isBlocked(&settingsBase, "+41448881111", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41448881111' blocked") == 0);
    assert(block->isBlocked(&settingsBase, "0448881234", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41448881234' blocked") == 0);
    assert(block->isBlocked(&settingsBase, "+23412312312", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+23412312312' blocked") == 0 ||
           msg.compare("Incoming call: number='+23412312312' blocked invalid") == 0);

    // test number in allowlist
    assert(block->isBlocked(&settingsBase, "0441112233", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112233' name='Mr. X 1' allowlist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+41441112233", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112233' name='Mr. X 1' allowlist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+4144222", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+4144222' name='Mr. X 3 too small' invalid allowlist='main'") == 0);

    // test number in allowlist and blocklist
    assert(block->isBlocked(&settingsBase, "+41441119999", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441119999' name='Mr. X 2 in blocklist too' allowlist='main'") == 0);

    // test: caller information from provider
    assert(block->isBlocked(&settingsBase, "+12025808292", "Mr. Spamer", &msg) == true);
    assert(msg.compare("Incoming call: number='+12025808292' name='Mr. Spamer' blocked") == 0);

    // test anonymous number
    settingsBase.blockAnonymousCID = false;
    assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == true);
    assert(msg.compare("Incoming call: number='anonymous' blocked") == 0);
    settingsBase.blockAnonymousCID = true;
    assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == true);
    assert(msg.compare("Incoming call: number='anonymous' blocked") == 0);

    // test invalid number: too small number
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+4144333' blocked invalid") == 0);
    settingsBase.blockInvalidCID = true;
    assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+4144333' blocked invalid") == 0);

    // test invalid number: too long number
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+493456789012345678' blocked invalid") == 0);
    settingsBase.blockInvalidCID = true;
    assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+493456789012345678' blocked invalid") == 0);

    // test name with wildcard
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+41441112222", "SPAM: yes", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112222' name='SPAM: yes' blocklist='main'") == 0);
}

static void TestCase_allowlists_and_blocklists(std::string etcPath)
{
    //printf("etc: %s\n", etcPath.c_str());
    Settings* settings = new Settings(etcPath);
    Block* block = new Block(settings);

    SettingBase settingsBase;
    settingsBase.blockAnonymousCID = false;
    settingsBase.blockInvalidCID = false;
    settingsBase.blockMode = ALLOWLISTS_AND_BLOCKLISTS;
    // CH
    settingsBase.countryCode = "+41";
    std::string msg;
    // test number in blocklist
    assert(block->isBlocked(&settingsBase, "0449999999", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+41449999999", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blocklist='main'") == 0);

    // test number in range in blocklist
    assert(block->isBlocked(&settingsBase, "0448881111", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocked blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+41448881111", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocked blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "0448881234", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41448881234' name='Test range 1' blocked blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+23412312312", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+23412312312' name='Test range 2' blocked blocklist='main'") == 0 ||
           msg.compare("Incoming call: number='+23412312312' name='Test range 2' blocked invalid blocklist='main'") == 0);

    // test number in allowlist
    assert(block->isBlocked(&settingsBase, "0441112233", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112233' name='Mr. X 1' allowlist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+41441112233", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112233' name='Mr. X 1' allowlist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+4144222", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+4144222' name='Mr. X 3 too small' invalid allowlist='main'") == 0);

    // test number in allowlist and blocklist
    assert(block->isBlocked(&settingsBase, "+41441119999", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441119999' name='Mr. X 2 in blocklist too' allowlist='main'") == 0);

    // test: caller information from provider
    assert(block->isBlocked(&settingsBase, "+12025808292", "Mr. Spamer", &msg) == false);
    assert(msg.compare("Incoming call: number='+12025808292' name='Mr. Spamer'") == 0);

    // test anonymous number
    settingsBase.blockAnonymousCID = false;
    assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == false);
    assert(msg.compare("Incoming call: number='anonymous'") == 0);
    settingsBase.blockAnonymousCID = true;
    assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == true);
    assert(msg.compare("Incoming call: number='anonymous' blocked") == 0);

    // test invalid number: too small number
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+4144333' invalid") == 0);
    settingsBase.blockInvalidCID = true;
    assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+4144333' blocked invalid") == 0);

    // test invalid number: too long number
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+493456789012345678' invalid") == 0);
    settingsBase.blockInvalidCID = true;
    assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+493456789012345678' blocked invalid") == 0);

    // test name with wildcard
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+41441112222", "SPAM: yes", &msg) == true);
    assert(msg.compare("Incoming call: number='+41441112222' name='SPAM: yes' blocked blocklist='main'") == 0);
}

static void TestCase_blocklists_only(std::string etcPath)
{
    //printf("etc: %s\n", etcPath.c_str());
    Settings* settings = new Settings(etcPath);
    Block* block = new Block(settings);

    SettingBase settingsBase;
    settingsBase.blockAnonymousCID = false;
    settingsBase.blockInvalidCID = false;
    settingsBase.blockMode = BLOCKLISTS_ONLY;
    // CH
    settingsBase.countryCode = "+41";
    std::string msg;
    // test number in blocklist
    assert(block->isBlocked(&settingsBase, "0449999999", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+41449999999", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41449999999' name='Test single number' blocked blocklist='main'") == 0);

    // test range in blocklist
    assert(block->isBlocked(&settingsBase, "0448881111", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocked blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+41448881111", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41448881111' name='Test range 1' blocked blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "0448881234", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41448881234' name='Test range 1' blocked blocklist='main'") == 0);
    assert(block->isBlocked(&settingsBase, "+23412312312", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+23412312312' name='Test range 2' blocked blocklist='main'") == 0 ||
           msg.compare("Incoming call: number='+23412312312' name='Test range 2' blocked invalid blocklist='main'") == 0);

    // test number in allowlist
    assert(block->isBlocked(&settingsBase, "0441112233", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112233'") == 0);
    assert(block->isBlocked(&settingsBase, "+41441112233", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+41441112233'") == 0);
    assert(block->isBlocked(&settingsBase, "+4144222", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+4144222' invalid") == 0);

    // test number in allowlist and blocklist
    assert(block->isBlocked(&settingsBase, "+41441119999", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+41441119999' name='Mr. X 2 in blocklist too' blocked blocklist='main'") == 0);

    // test: caller information from provider
    assert(block->isBlocked(&settingsBase, "+12025808292", "Mr. Spamer", &msg) == false);
    assert(msg.compare("Incoming call: number='+12025808292' name='Mr. Spamer'") == 0);

    // test anonymous number
    settingsBase.blockAnonymousCID = false;
    assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == false);
    assert(msg.compare("Incoming call: number='anonymous'") == 0);
    settingsBase.blockAnonymousCID = true;
    assert(block->isBlocked(&settingsBase, "anonymous", "", &msg) == true);
    assert(msg.compare("Incoming call: number='anonymous' blocked") == 0);

    // test invalid number: too short number
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+4144333' invalid") == 0);
    settingsBase.blockInvalidCID = true;
    assert(block->isBlocked(&settingsBase, "+4144333", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+4144333' blocked invalid") == 0);

    // test invalid number: too long number
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == false);
    assert(msg.compare("Incoming call: number='+493456789012345678' invalid") == 0);
    settingsBase.blockInvalidCID = true;
    assert(block->isBlocked(&settingsBase, "+493456789012345678", "", &msg) == true);
    assert(msg.compare("Incoming call: number='+493456789012345678' blocked invalid") == 0);

    // test name with wildcard
    settingsBase.blockInvalidCID = false;
    assert(block->isBlocked(&settingsBase, "+41441112222", "SPAM: yes", &msg) == true);
    assert(msg.compare("Incoming call: number='+41441112222' name='SPAM: yes' blocked blocklist='main'") == 0);
}

void Test_Block_Run(std::string etcPath)
{
    printf("Test_Block_Run...\n");
    Logger::setLogLevel(LogLevel::WARN);

    TestCase_logging_only(etcPath);
    TestCase_allowlists_only(etcPath);
    TestCase_allowlists_and_blocklists(etcPath);
    TestCase_blocklists_only(etcPath);
}
