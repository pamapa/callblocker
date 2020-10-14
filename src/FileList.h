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

#ifndef FILELIST_H
#define FILELIST_H

#include <chrono>
#include <string>
#include <vector>

struct FileListEntry {
    std::string number;
    std::string name;
    std::chrono::system_clock::time_point date_created;
};

class FileList {
private:
    std::string m_filename;
    std::string m_name;
    std::vector<FileListEntry> m_entries;

public:
    FileList(const std::string& filename, const std::string& name);
    FileList(const std::string& filename);
    virtual ~FileList();

    bool load();
    bool save();

    std::string getName();

    bool getEntryByNumber(const std::string& rNumber, std::string* pName);
    bool getEntryByName(const std::string& rName);
    void addEntry(const std::string& rNumber, const std::string& rName);
    void removeEntry(const std::string& rNumber);

    bool eraseAged(size_t maxDays);

    void dump();
};

#endif
