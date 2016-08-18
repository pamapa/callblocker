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

#include "Utils.h"


static void TestCase_string()
{
  // startsWith
  assert(Utils::startsWith("+212122112", "+") == true);
  assert(Utils::startsWith("+212122112", "-") == false);

  // trim
  std::string str = "123";
  Utils::trim(str);
  assert(str.compare("123") == 0);
  str = "  123";
  Utils::trim(str);
  assert(str.compare("123") == 0);
  str = "123  ";
  Utils::trim(str);
  assert(str.compare("123") == 0);
  str = "  123  ";
  Utils::trim(str);
  assert(str.compare("123") == 0);
  str = "\t\n123\n\n\n";
  Utils::trim(str);
  assert(str.compare("123") == 0);
}

void Test_Utils_Run()
{
  printf("Test_Utils_Run...\n");
  
  TestCase_string();
}

