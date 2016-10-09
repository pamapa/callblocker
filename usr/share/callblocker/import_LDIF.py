#!/usr/bin/env python

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2016 Patrick Ammann <pammann@gmx.net>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

from __future__ import print_function
import re
from ldif import LDIFParser
from datetime import datetime

from import_base import ImportBase


class MyLDIF(LDIFParser):
    def __init__(self, input, log):
        LDIFParser.__init__(self, input)
        self.log = log
        self.date = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S +0000")
        self.entries = []

    def _extract_number(self, data):
        n = re.sub(r"[^0-9\+]", "", data)
        return n

    def _get_entity_person(self, entry):
        fname = ""
        sname = ""
        cname = ""
        if "givenName" in entry: fname = entry["givenName"][0].decode(encoding='UTF-8', errors='strict')
        if "sn" in entry: sname = entry["sn"][0].decode(encoding='UTF-8', errors='strict')
        if "cn" in entry: cname = entry["cn"][0].decode(encoding='UTF-8', errors='strict')
        name = ""
        if sname == "": name = cname
        elif fname == "": name = sname
        else: name = fname + " " + sname
        return name

    def handle(self, dn, entry):
        self.log.debug(entry)
        name = self._get_entity_person(entry)
        if "mobile" in entry:
            number = self._extract_number(entry["mobile"][0])
            field_name = "Mobile Phone"
            if len(number) != 0:
                self.entries.append({"number": number, "name": name + " (" + field_name + ")",
                                     "date_created": self.date, "date_modified": self.date})
        if "homePhone" in entry:
            number = self._extract_number(entry["homePhone"][0])
            field_name = "Home Phone"
            if len(number) != 0:
                self.entries.append({"number": number, "name": name + " (" + field_name + ")",
                                     "date_created": self.date, "date_modified": self.date})
        if "telephoneNumber" in entry:
            number = self._extract_number(entry["telephoneNumber"][0])
            field_name = "Work Phone"
            if len(number) != 0:
                self.entries.append({"number": number, "name": name + " (" + field_name + ")",
                                     "date_created": self.date, "date_modified": self.date})


class ImportLDIF(ImportBase):
    def get_entries(self, args):
        parser = MyLDIF(open(args.input, "r"), self.log)
        parser.parse()
        return parser.entries


# main
#
if __name__ == "__main__":
    m = ImportLDIF()
    parser = m.get_parser("Convert LDIF file to json")
    args = parser.parse_args()
    m.run(args)
