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
#include <assert.h>

#include "Settings.h"
#include "Utils.h"


static void TestCase_string()
{
  // startsWith
  assert(Utils::startsWith("+212122112", "+") == true);
  assert(Utils::startsWith("+212122112", "-") == false);

  // trim
  std::string str = "123";
  Utils::trim(&str);
  assert(str.compare("123") == 0);

  str = "  123";
  Utils::trim(&str);
  assert(str.compare("123") == 0);

  str = "123  ";
  Utils::trim(&str);
  assert(str.compare("123") == 0);

  str = "  123  ";
  Utils::trim(&str);
  assert(str.compare("123") == 0);

  str = "\t\n123\n\n\n";
  Utils::trim(&str);
  assert(str.compare("123") == 0);
}

static void TestCase_makeNumberInternational()
{
  SettingBase settings;
  // CH
  settings.countryCode = "+41";
  std::string str = "0441234567"; // local
  bool valid;
  Utils::makeNumberInternational(&settings, &str, &valid);
  assert(str.compare("+41441234567") == 0);
  assert(valid);
  
  str = "+41791234567"; // already international
  Utils::makeNumberInternational(&settings, &str, &valid);
  assert(str.compare("+41791234567") == 0);
  assert(valid);
  
  str = "0041791234567"; // already international
  Utils::makeNumberInternational(&settings, &str, &valid);
  assert(str.compare("+41791234567") == 0);
  assert(valid);
}

static void TestCase_parseCallerID()
{
  std::vector<std::pair<std::string, std::string>> result;
  std::string str = "DATE=0306\nTIME=1517\nNMBR=0123456789\nNAME=aasd asdd\n";
  Utils::parseCallerID(str, &result);
  assert(result.size() == 4);
  assert(result[0].first.compare("DATE") == 0);
  assert(result[0].second.compare("0306") == 0);
  assert(result[1].first.compare("TIME") == 0);
  assert(result[1].second.compare("1517") == 0);
  assert(result[2].first.compare("NMBR") == 0);
  assert(result[2].second.compare("0123456789") == 0);
  assert(result[3].first.compare("NAME") == 0);
  assert(result[3].second.compare("aasd asdd") == 0);
  result.clear();

  str = "DATE\n TIME=12";
  Utils::parseCallerID(str, &result);
  assert(result.size() == 1);
  assert(result[0].first.compare("TIME") == 0);
  assert(result[0].second.compare("12") == 0);
  result.clear();

  str = "DATE= 12\nTIME=1\nNMBR=\nNAME=saas ";
  Utils::parseCallerID(str, &result);
  assert(result.size() == 4);
  assert(result[0].first.compare("DATE") == 0);
  assert(result[0].second.compare("12") == 0);
  assert(result[1].first.compare("TIME") == 0);
  assert(result[1].second.compare("1") == 0);
  assert(result[2].first.compare("NMBR") == 0);
  assert(result[2].second.compare("") == 0);
  assert(result[3].first.compare("NAME") == 0);
  assert(result[3].second.compare("saas") == 0);
  result.clear();
}

void Test_Utils_Run()
{
  printf("Test_Utils_Run...\n");
  
  TestCase_string();
  TestCase_makeNumberInternational();
  TestCase_parseCallerID();
}

