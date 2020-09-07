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

#include "Test.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <json-c/json.h>

#include "FileListsNotified.h"
#include "Logger.h"
#include "Utils.h"


static void checkEntry(FileListsNotified* pNotified,
                       const std::string& rNumber, bool exist, const std::string& rExpListName, const std::string& rExpCallerName) {
  //printf("checkEntry(rNumber='%s' exist=%d rExpListName='%s' rExpCallerName='%s')\n", rNumber.c_str(), (int)exist, rExpListName.c_str(), rExpCallerName.c_str());
  std::string listName, callerName;
  assert(pNotified->getEntry(rNumber, &listName, &callerName) == exist);
  if (exist) {
    assert(listName == rExpListName);
    assert(callerName == rExpCallerName);
  }
}

static void addEntry(std::string filename, const std::string& rNumber, const std::string& rExpCallerName) {
  FileList* pNotified = new FileList(filename);
  assert(pNotified->load());
  pNotified->addEntry(rNumber, rExpCallerName);
  assert(pNotified->save());
  delete(pNotified);
}

static void removeEntry(std::string filename, const std::string& rNumber) {
  FileList* pNotified = new FileList(filename);
  assert(pNotified->load());
  pNotified->removeEntry(rNumber);
  assert(pNotified->save());
  delete(pNotified);
}

static void Test_WithExistingOne(std::string etcPath) {
  char tmpl[] = "/tmp/testcallblockerd.XXXXXX";
  char* tempPath = mkdtemp(tmpl);
  assert(tempPath != NULL);  
  //printf("tempPath: %s\n", tempPath);
  std::string listPath = Utils::pathJoin(etcPath, "allowlists");

  // start with existing one
  assert(Utils::fileCopy(Utils::pathJoin(listPath, "main.json"), Utils::pathJoin(tempPath, "main.json")));

  FileListsNotified* pNotified = new FileListsNotified(tempPath);
  //pNotified->dump();
  checkEntry(pNotified, "+41441112233", true, "main", "Mr. X 1");
  checkEntry(pNotified, "+41441119999", true, "main", "Mr. X 2 in blocklist too");
  checkEntry(pNotified, "+4144222",     true, "main", "Mr. X 3 too small");

  // simulate "extern" append (what pyhon would do)
  addEntry(Utils::pathJoin(tempPath, "main.json"), "+11111111111", "Add Entry 1");
  pNotified->run();
  //pNotified->dump();
  checkEntry(pNotified, "+41441112233", true, "main", "Mr. X 1");
  checkEntry(pNotified, "+41441119999", true, "main", "Mr. X 2 in blocklist too");
  checkEntry(pNotified, "+4144222",     true, "main", "Mr. X 3 too small");
  checkEntry(pNotified, "+11111111111", true, "main", "Add Entry 1");
  
  // simulate "extern" remove (what pyhon would do)
  removeEntry(Utils::pathJoin(tempPath, "main.json"), "+41441112233");
  pNotified->run();
  //pNotified->dump();
  checkEntry(pNotified, "+41441112233", false, "", "");
  checkEntry(pNotified, "+41441119999", true, "main", "Mr. X 2 in blocklist too");
  checkEntry(pNotified, "+4144222",     true, "main", "Mr. X 3 too small");
  checkEntry(pNotified, "+11111111111", true, "main", "Add Entry 1");
  
  delete(pNotified);

  remove(Utils::pathJoin(tempPath, "main.json").c_str());
  remove(tempPath);
}

void Test_FileListsNotified_Run(std::string etcPath) {
  printf("Test_FileListsNotified_Run...\n");
  Logger::setLogLevel(LogLevel::WARN);

  Test_WithExistingOne(etcPath);
}

