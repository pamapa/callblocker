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

#include <string>
#include <stdio.h>
#include <assert.h>

#include "Test.h"
#include "Utils.h"
#include "Logger.h"


int main(int argc, char* argv[]) {
  printf("Executing unit_tests...\n");

  Logger::start(false);

#if 0
  assert(0);
#endif

  std::string exePath = Utils::pathDirname(Utils::pathAbsname(argv[0]));
  //printf("exePath: %s\n", exePath.c_str());

  Logger::setLogLevel(LogLevel::WARN);
  Test_Utils_Run();
  Logger::setLogLevel(LogLevel::WARN);
  Test_Block_Run(exePath);
  Logger::setLogLevel(LogLevel::WARN);
  Test_FileListsCache_Run(exePath);
  Logger::setLogLevel(LogLevel::WARN);
  Test_FileListsNotified_Run(exePath);

  Logger::stop();

  return 0; // success (asserts will abort before)
}

