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

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <json-c/json.h>

#include "FileListsNotified.h"
#include "Logger.h"
#include "Utils.h"


static void checkEntryByNumber(FileListsNotified* pNotified, const std::string& rNumber,
                               bool exist, const std::string& rExpListName, const std::string& rExpCallerName) {
  //fprintf(stderr, "checkEntry(rNumber='%s' exist=%d rExpListName='%s' rExpCallerName='%s')\n", rNumber.c_str(), (int)exist, rExpListName.c_str(), rExpCallerName.c_str());
  std::string listName, callerName;
  assert(pNotified->getEntryByNumber(rNumber, &listName, &callerName) == exist);
  if (exist) {
    assert(listName == rExpListName);
    assert(callerName == rExpCallerName);
  }
}

static void checkEntryByName(FileListsNotified* pNotified, const std::string& rName,
                             bool exist, const std::string& rExpListName) {
  //fprintf(stderr, "checkEntry(rName='%s' exist=%d rExpListName='%s')\n", rName.c_str(), (int)exist, rExpListName.c_str());
  std::string listName, callerName;
  assert(pNotified->getEntryByName(rName, &listName) == exist);
  if (exist) {
    assert(listName == rExpListName);
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
  assert(tempPath != nullptr);
  //fprintf(stderr, "tempPath: %s\n", tempPath);
  std::string listPath = Utils::pathJoin(etcPath, "blocklists");

  // start with existing one
  assert(Utils::fileCopy(Utils::pathJoin(listPath, "main.json"), Utils::pathJoin(tempPath, "main.json")));

  FileListsNotified* pNotified = new FileListsNotified(tempPath);
  //pNotified->dump();
  checkEntryByNumber(pNotified, "+41449999999", true, "main", "Test single number");
  checkEntryByNumber(pNotified, "+4144888", true, "main", "Test range 1");
  checkEntryByNumber(pNotified, "+234",     true, "main", "Test range 2");
  checkEntryByName(pNotified, "SPAM 123", true, "main");
  checkEntryByName(pNotified, "Test single number", false, "main"); // number entry

  // simulate "extern" append (what pyhon would do)
  addEntry(Utils::pathJoin(tempPath, "main.json"), "+11111111111", "Add Entry 1");
  pNotified->run();
  //pNotified->dump();
  checkEntryByNumber(pNotified, "+41449999999", true, "main", "Test single number");
  checkEntryByNumber(pNotified, "+4144888", true, "main", "Test range 1");
  checkEntryByNumber(pNotified, "+234",     true, "main", "Test range 2");
  checkEntryByNumber(pNotified, "+11111111111", true, "main", "Add Entry 1");
  
  // simulate "extern" remove (what pyhon would do)
  removeEntry(Utils::pathJoin(tempPath, "main.json"), "+41449999999");
  pNotified->run();
  //pNotified->dump();
  checkEntryByNumber(pNotified, "+41449999999", false, "", "");
  checkEntryByNumber(pNotified, "+4144888", true, "main", "Test range 1");
  checkEntryByNumber(pNotified, "+234",     true, "main", "Test range 2");
  checkEntryByNumber(pNotified, "+11111111111", true, "main", "Add Entry 1");

  delete(pNotified);
  remove(Utils::pathJoin(tempPath, "main.json").c_str());
  remove(tempPath);
}

void Test_FileListsNotified_Run(std::string etcPath) {
  printf("Test_FileListsNotified_Run...\n");
  Logger::setLogLevel(LogLevel::WARN);

  Test_WithExistingOne(etcPath);
}
