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
import os, sys, re
from BeautifulSoup import BeautifulSoup
from datetime import datetime
from collections import OrderedDict

from blacklist_base import BlacklistBase


class BlacklistKTippCH(BlacklistBase):

    def _extract_number(self, data):
        n = re.sub(r"[^0-9\+]","", data)
        return n

    # 021 558 73 91/92/93/94/95 or
    # 0099 9910 0450/0451/0453
    # -> 009999100450
    def _extract_slashed_numbers(self, data):
        ret = []
        arr = data.split("/")
        a0 = self._extract_number(arr[0])
        if a0 != "" and len(arr) > 1:
            ret.append(a0)
            slash_len = len(arr[1])
            base = a0[0:-slash_len]
            for ax in arr[1:]:
                ax = self._extract_number(ax)
                if ax != "":
                    ax = self._extract_number(base + ax)
                    ret.append(ax)
        return ret

    # 044 400 00 00 bis 044 400 00 19
    def _extract_range_numbers(self, data):
        ret = []
        arr = re.split("bis", data)
        s = self._extract_number(arr[0])
        e = self._extract_number(arr[1])
        for i in range(int(s[-4:]), int(e[-4:])+1):
            a = s[:-4]+"%04d" % i
            ret.append(a)
        return ret

    def _extract_numbers(self, data):
        ret = []
        arr = re.split("und|oder|sowie|auch|,|;", data)
        for a in arr:
            if a.find("/") != -1:
                ret.extend(self._extract_slashed_numbers(a))
            elif a.find("bis") != -1:
                ret.extend(self._extract_range_numbers(a))
            else:
                a = self._extract_number(a)
                if a != "": ret.append(a)
        self.log.debug("_extract_numbers() data:'%s' -> %s" % (data, ret))
        return ret

    def _extract_name(self, data):
        s = unicode(data)
        s = s.replace("\n", "").replace("\r", "")
        s = re.sub(r'<[^>]*>', " ", s) # remove tags
        s = s.replace("&amp", "&")
        s = s.replace("  ", " ")
        s = s.strip()
        if s.startswith("Firma: "): s = s[7:]
        return self.minimize_name(s)

    def _fetch_page(self, page_nr):
        print("fetch_page: " + str(page_nr))
        url = "https://www.ktipp.ch/service/warnlisten/detail/?warnliste_id=7&ajax=ajax-search-form&page=" + str(page_nr)
        return self.http_get(url)

    def _extract_str(self, data, start_str, end_str, error_msg):
        s = data.find(start_str)
        if s == -1:
            self.log.error(error_msg+". Start ("+start_str+") not found.")
            sys.exit(-1)
        s += len(start_str)
        e = data.find(end_str, s)
        if e == -1:
            self.log.error(error_msg+". End ("+end_str+") not found.")
            sys.exit(-1)
        return data[s:e].strip()

    def _parse_page(self, soup):
        ret = []
        #self.log.debug("parse_page...")
        number_list = soup.findAll("section", {"class": "teaser cf"})
        now = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S +0000")
        for e in number_list:
            numbers = self._extract_numbers(e.strong.contents[0])
            name = self._extract_name(e.p)
            for n in numbers:
                ret.append({"number": n, "name": name, "date_created": now, "date_modified": now})
        #self.log.debug("parse_page done")
        return ret

    def _parse_pages(self, content):
        ret = []

        soup = BeautifulSoup(content)
        tmp = str(soup.findAll("li")[-1])
        max_page_str = self._extract_str(tmp, "ajaxPagerWarnlisteLoadIndex(", ")", "Can't extract max pages")
        last_page = int(max_page_str)
        #print last_page
        ret.extend(self._parse_page(soup))

        # only first page
        #return ret

        for p in range(1, last_page+1):
            content = self._fetch_page(p)
            soup = BeautifulSoup(content)
            #self.log.debug(soup)
            ret.extend(self._parse_page(soup))
        return ret

    def get_result(self, args, last_update):

        content = self._fetch_page(0)
        source_date = unicode(self._extract_str(content, "Letzte Aktualisierung:", "<", "Can't extract creation date"))
        self.log.debug(source_date)
        if last_update == source_date:
            # we already have this version
            self.log.debug("We already have this version")
            sys.exit(0)

        entries = self._parse_pages(content)
        entries = self.cleanup_entries(entries, country_code="+41")

        result = OrderedDict()
        result["name"] = "ktipp.ch blacklist"
        result["origin"] = "https://www.ktipp.ch/service/warnlisten/detail/?warnliste_id=7"
        result["parsed_by"] = "callblocker script: %s" % os.path.basename(__file__)
        result["last_update"] = source_date
        result["num_entries"] = len(entries)
        result["entries"] = entries
        return result


#
# main
#
if __name__ == "__main__":
    m = BlacklistKTippCH()
    parser = m.get_parser("Fetch blacklist provided by ktipp.ch")
    args = parser.parse_args()
    json_filename = args.output + "/blacklist_ktipp_ch.json"
    m.run(args, json_filename)
