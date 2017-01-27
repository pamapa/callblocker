#!/usr/bin/env python

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2017 Patrick Ammann <pammann@gmx.net>
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
import sys, argparse
import logging
import urllib2
import json


class OnlineBase(object):

    def __init__(self):
        logging.basicConfig()
        self.log = logging.getLogger()
        self.log.setLevel(logging.WARN)

    def get_parser(self, description):
        parser = argparse.ArgumentParser(description=description)
        parser.add_argument("--number", help="phone number to be checked", required=True)
        parser.add_argument('--debug', action="store_true")
        return parser

    def http_get(self, url, add_headers={}, allowed_codes=[]):
        self.log.debug("http_get: '%s'" % url)
        try:
            request = urllib2.Request(url, headers=add_headers)
            response = urllib2.urlopen(request, timeout=5)
            data = response.read()
        except urllib2.HTTPError, e:
            code = e.getcode()
            if code not in allowed_codes:
                raise
            data = e.read()
        return data

    # must be implemented in the inherited class
    def handle_number(self, args, number):
        return ""

    # must be implemented in the inherited class
    def supported_country_codes(self):
        return []

    def onlinelookup_2_result(self, caller_name):
        if not isinstance(caller_name, unicode):
            self.log.warn("caller_name must have type unicode")
        result = {
            "name": caller_name
        }
        return result

    def onlinecheck_2_result(self, spam, score, caller_name=""):
        result = {
            "spam": spam,
            "score": score,
            "name": caller_name
        }
        return result

    def run(self, args):
        if args.debug: self.log.setLevel(logging.DEBUG)

        supported = False
        for cc in self.supported_country_codes():
            if args.number.startswith(cc):
                supported = True
                break
        if not supported:
            self.log.error("number '%s' is not supported '%s'" % (args.number, self.supported_country_codes()))
            sys.exit(-1)

        result = self.handle_number(args, args.number)

        # result in json format, if not found: empty fields
        j = json.dumps(result, encoding="utf-8", ensure_ascii=False)
        sys.stdout.write(j.encode("utf8"))
        sys.stdout.write("\n")  # must be separate line, to avoid conversion of json into ascii

        # no error occurred
        sys.exit(0)
