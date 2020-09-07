#!/usr/bin/env python3

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2019 Patrick Ammann <pammann@gmx.net>
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

import os, sys, re
from bs4 import BeautifulSoup
from datetime import datetime
from collections import OrderedDict

from blocklist_base import BlocklistBase


class BlocklistKTippCH(BlocklistBase):

    def _extract_number(self, data):
        n = re.sub(r"[^0-9\+]","", data)
        return n

    def _extract_name(self, data):
        s = data
        s = s.replace("\n", "").replace("\r", "")
        s = re.sub(r'<[^>]*>', " ", s) # remove tags
        s = s.replace("&amp", "&")
        s = s.replace("  ", " ")
        s = s.strip()
        if s.startswith("Firma: "): s = s[7:]
        # self.log.debug("_extract_name() data:'%s' -> '%s'" % (data, s))
        return s

    def _fetch_page(self, page_nr):
        # print("fetch_page: " + str(page_nr))
        url = "https://www.ktipp.ch/service/warnlisten/detail/warnliste/unerwuenschte-oder-laestige-telefonanrufe/"
        url += "?tx_updkonsuminfo_konsuminfofe[%40widget_0][currentPage]=" + str(page_nr)
        ret = self.http_get(url)
        # print("%s\n%s\n%s" % ("-"*80, ret, "-"*80))
        return ret

    def _parse_page(self, soup):
        ret = []
        # self.log.debug("parse_page...")
        content = soup.find("div", id="warnlisteContent")
        number_list = content.findAll("article")
        now = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S +0000")
        for e in number_list:
            number = self._extract_number(e.find("h3").get_text())
            e.h3.decompose()  # remove h3
            name = self._extract_name(str(e))
            self.log.debug("number:'%s' name:'%s'" % (number, name))
            ret.append({"number": number, "name": name, "date_created": now, "date_modified": now})
        # self.log.debug("parse_page done")
        return ret

    def _parse_pages(self, last_update):
        ret = []

        content = self._fetch_page(1)
        soup = BeautifulSoup(content, "lxml")
        ret.extend(self._parse_page(soup))

        # already parsed?
        current_update = ret[0]["number"]  # newest added number
        self.log.debug("Current update: '%s'" % current_update)
        if last_update == current_update:
            # we already have this version
            self.log.debug("We already have this version")
            sys.exit(0)

        # find last page
        tmp = soup.find("div", id="warnlisteContent")
        tmp = tmp.findAll("li")[-2]
        a = tmp.find("a", href=True)
        last_page = int(a.string)
        self.log.debug("last_page: %d" % last_page)
        # last_page = 1

        for p in range(2, last_page + 1):
            content = self._fetch_page(p)
            soup = BeautifulSoup(content, "lxml")
            ret.extend(self._parse_page(soup))
            # print("entries: %d" % len(ret))
        return ret, current_update

    def get_result(self, args, last_update):
        entries, current_update = self._parse_pages(last_update)
        entries = self.cleanup_entries(entries, country_code="+41")

        result = OrderedDict()
        result["name"] = "ktipp.ch blocklist"
        result["origin"] = "https://www.ktipp.ch/service/warnlisten/detail/warnliste/unerwuenschte-oder-laestige-telefonanrufe/"
        result["parsed_by"] = "callblocker script: %s" % os.path.basename(__file__)
        result["last_update"] = current_update
        result["num_entries"] = len(entries)
        result["entries"] = entries
        return result


#
# main
#
if __name__ == "__main__":
    m = BlocklistKTippCH()
    parser = m.get_parser("Fetch blocklist provided by ktipp.ch")
    args = parser.parse_args()
    json_filename = args.output + "/blocklist_ktipp_ch.json"
    m.run(args, json_filename)
