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

#include "FileListsCache.h"
#include "Logger.h"
#include "Utils.h"


static void checkEntry(FileListsCache* pCache, const CacheType type,
                       const std::string& rNumber, bool exist, const std::string& rExpCallerName) {
  //printf("checkEntry(rNumber='%s' exist=%d rExpCallerName='%s')\n", rNumber.c_str(), (int)exist, rExpCallerName.c_str());
  std::string callerName;
  assert(pCache->getEntry(type, rNumber, &callerName) == exist);
  if (exist) {
    assert(callerName == rExpCallerName);
  }
}

static void Test_Empty(std::string etcPath) {
  char tmpl[] = "/tmp/testcallblockerd.XXXXXX";
  char* tempPath = mkdtemp(tmpl);
  assert(tempPath != NULL);  
  //printf("tempPath: %s\n", tempPath);

  // start empty
  FileListsCache* pCache = new FileListsCache(tempPath);
  //pCache->dump();

  // OnlineLookup: append
  pCache->addEntry(CacheType::OnlineLookup, "+22222221", "Entry Add 1");
  pCache->addEntry(CacheType::OnlineLookup, "+22222222", "Entry Add 2");
  //pCache->dump();
  checkEntry(pCache, CacheType::OnlineLookup, "+22222221", true, "Entry Add 1");
  checkEntry(pCache, CacheType::OnlineLookup, "+22222222", true, "Entry Add 2");

  // OnlineLookup: append
  pCache->addEntry(CacheType::OnlineCheck, "+33333331", "Check Add 1");
  pCache->addEntry(CacheType::OnlineCheck, "+33333332", "Check Add 2");
  //pCache->dump();
  checkEntry(pCache, CacheType::OnlineCheck, "+33333331", true, "Check Add 1");
  checkEntry(pCache, CacheType::OnlineCheck, "+33333332", true, "Check Add 2");

  pCache->run();
  //pCache->dump();
  checkEntry(pCache, CacheType::OnlineLookup, "+22222221", true, "Entry Add 1");
  checkEntry(pCache, CacheType::OnlineLookup, "+22222222", true, "Entry Add 2");
  checkEntry(pCache, CacheType::OnlineCheck, "+33333331", true, "Check Add 1");
  checkEntry(pCache, CacheType::OnlineCheck, "+33333332", true, "Check Add 2");

  delete(pCache);

  remove(Utils::pathJoin(tempPath, "onlinelookup.json").c_str());
  remove(Utils::pathJoin(tempPath, "onlinecheck.json").c_str());
  remove(tempPath);
}

static void Test_WithAged(std::string etcPath) {

  char tmpl[] = "/tmp/testcallblockerd.XXXXXX";
  char* tempPath = mkdtemp(tmpl);
  assert(tempPath != NULL);  
  //printf("tempPath: %s\n", tempPath);
  std::string cachePath = Utils::pathJoin(etcPath, "cache");

  // procondtion: already contains some old entries
  assert(Utils::fileCopy(Utils::pathJoin(cachePath, "onlinelookup.json"), Utils::pathJoin(tempPath, "onlinelookup.json")));
  assert(Utils::fileCopy(Utils::pathJoin(cachePath, "onlinecheck.json"), Utils::pathJoin(tempPath, "onlinecheck.json")));

  FileListsCache* pCache = new FileListsCache(tempPath);
  //pCache->dump();
  checkEntry(pCache, CacheType::OnlineLookup, "+11111111", true, "Entry Get 1");
  checkEntry(pCache, CacheType::OnlineLookup, "+11111112", true, "Entry Get 2");

  // append
  pCache->addEntry(CacheType::OnlineLookup, "+22222221", "Entry Add 1");
  pCache->addEntry(CacheType::OnlineLookup, "+22222222", "Entry Add 2");
  //pCache->dump();
  checkEntry(pCache, CacheType::OnlineLookup, "+11111111", true, "Entry Get 1");
  checkEntry(pCache, CacheType::OnlineLookup, "+11111112", true, "Entry Get 2");
  checkEntry(pCache, CacheType::OnlineLookup, "+22222221", true, "Entry Add 1");
  checkEntry(pCache, CacheType::OnlineLookup, "+22222222", true, "Entry Add 2");

  // age
  pCache->run();
  //pCache->dump();
  checkEntry(pCache, CacheType::OnlineLookup, "+11111111", false, "");
  checkEntry(pCache, CacheType::OnlineLookup, "+11111112", false, "");
  checkEntry(pCache, CacheType::OnlineLookup, "+22222221", true, "Entry Add 1");
  checkEntry(pCache, CacheType::OnlineLookup, "+22222222", true, "Entry Add 2");

  delete(pCache);

  remove(Utils::pathJoin(tempPath, "onlinelookup.json").c_str());
  remove(Utils::pathJoin(tempPath, "onlinecheck.json").c_str());
  remove(tempPath);
}

void Test_FileListsCache_Run(std::string etcPath) {
  printf("Test_FileListsCache_Run...\n");
  Logger::setLogLevel(LogLevel::WARN);

  Test_Empty(etcPath);
  Test_WithAged(etcPath);
}

