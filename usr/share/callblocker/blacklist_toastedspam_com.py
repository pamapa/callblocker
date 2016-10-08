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
import os, sys, argparse, re
from BeautifulSoup import BeautifulSoup
import urllib2
from collections import OrderedDict
from datetime import datetime
import json

from blacklist_base import BlacklistBase


NAME_MAX_LENGTH = 200


class BlacklistKToastedSpamCOM(BlacklistBase):

    def _extract_number(self, data):
        n = re.sub(r"[^0-9\+]","", data)
        return n

    def _extract_numbers(self, data):
        ret = []
        #print "data:" + data

        # 514-931-2572x110 -> 514-931-2572
        x = data.find("x")
        if x != -1: data = data[0:x]

        a = self._extract_number(data)
        if a != "": ret.append(a)
        return ret

    def _extract_name(self, data):
        s = data
        if s.startswith("- "): s = s[2:]
        s = s.replace("  ", " ")
        s = s.strip()
        return s if len(s)<= NAME_MAX_LENGTH else s[0:NAME_MAX_LENGTH-3]+"..."

    def _parse_page(self, content):
        ret = []
        soup = BeautifulSoup(content)
        #self.log.debug(soup)
        list = soup.findAll("b")
        now = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S +0000")
        for e in list:
            numbers = self._extract_numbers(e.contents[0])
            name = self._extract_name(e.nextSibling)
            for n in numbers:
                ret.append({"number": n, "name": name, "date_created": now, "date_modified": now})
        return ret

    # remove duplicates
    # remove too small numbers -> dangerous
    # make sure numbers are in international format (e.g. +41AAAABBBBBB)
    def _cleanup_entries(self, arr):
        self.log.debug("cleanup_entries (num=%s)" % len(arr))
        seen = set()
        uniq = []
        for r in arr:
            x = r["number"]

            # make international format
            if not x.startswith("+"):  x = "+1"+x
            r["number"] = x

            # filter
            if len(x) < 4:
                # too dangerous
                self.log.debug("Skip too small number: " + str(r))
                continue
            if len(x) > 16:
                # see spec E.164 for international numbers: 15 (including country code) + 1 ("+")
                self.log.debug("Skip too long number:" + str(r))
                continue;

            if x not in seen:
                uniq.append(r)
                seen.add(x)
        self.log.debug("cleanup_entries done (num=%s)" % len(uniq))
        return uniq

    def get_result(self, args, last_update):
        content = self.http_get("http://www.toastedspam.com/phonelist.cgi")

        entries = self._parse_page(content)
        entries = self._cleanup_entries(entries)

        result = OrderedDict()
        result["name"] = "toastedspam.com blacklist"
        result["origin"] = "http://www.toastedspam.com/phonelist.cgi"
        result["parsed_by"] = "callblocker script: %s" % os.path.basename(__file__)
        result["num_entries"] = len(entries)
        result["entries"] = entries
        return result


#
# main
#
if __name__ == "__main__":
    m = BlacklistKToastedSpamCOM()
    parser = m.get_parser("Fetch blacklist provided by toastedspam.com")
    args = parser.parse_args()
    json_filename = args.output + "/blacklist_toastedspam_com.json"
    m.run(args, json_filename)
