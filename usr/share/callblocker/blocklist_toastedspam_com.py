#!/usr/bin/env python3

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2020 Patrick Ammann <pammann@gmx.net>
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

import os, re
from bs4 import BeautifulSoup
from collections import OrderedDict
from datetime import datetime

from blocklist_base import BlocklistBase


class BlocklistKToastedSpamCOM(BlocklistBase):

    def _extract_number(self, data):
        n = re.sub(r"[^0-9\+]", "", data)
        return n

    def _extract_numbers(self, data):
        ret = []
        #print "data:" + data

        # 514-931-2572x110 -> 514-931-2572
        x = data.find("x")
        if x != -1: data = data[0:x]

        # 865-362-4450pin4346 -> 865-362-4450
        x = data.find("pin")
        if x != -1: data = data[0:x]

        # no idea
        x = data.find("fastsize")
        if x != -1: data = ""

        a = self._extract_number(data)
        if a != "": ret.append(a)
        self.log.debug("_extract_numbers() data:'%s' -> %s" % (data, ret))
        return ret

    def _extract_name(self, data):
        s = data
        if s.startswith("- "): s = s[2:]
        s = s.replace("  ", " ")
        s = s.strip()
        return self.minimize_name(s)

    def _parse_page(self, content):
        ret = []
        soup = BeautifulSoup(content, "lxml")
        #self.log.debug(soup)
        number_list = soup.findAll("b")
        now = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S +0000")
        for e in number_list:
            numbers = self._extract_numbers(e.contents[0].strip())
            name = self._extract_name(e.nextSibling.strip())
            for n in numbers:
                ret.append({"number": n, "name": name, "date_created": now, "date_modified": now})
        return ret

    def get_result(self, args, last_update):
        content = self.http_get("http://www.toastedspam.com/phonelist.cgi")

        entries = self._parse_page(content)
        entries = self.cleanup_entries(entries, country_code="+1")

        result = OrderedDict()
        result["name"] = "toastedspam.com blocklist"
        result["origin"] = "http://www.toastedspam.com/phonelist.cgi"
        result["parsed_by"] = "callblocker script: %s" % os.path.basename(__file__)
        result["num_entries"] = len(entries)
        result["entries"] = entries
        return result


#
# main
#
if __name__ == "__main__":
    m = BlocklistKToastedSpamCOM()
    parser = m.get_parser("Fetch blocklist provided by toastedspam.com")
    args = parser.parse_args()
    json_filename = args.output + "/blocklist_toastedspam_com.json"
    m.run(args, json_filename)
