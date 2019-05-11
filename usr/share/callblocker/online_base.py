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

import sys, argparse
import logging
import urllib.request
import json
import socket


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
        self.log.debug("http_get('%s',%s,%s)" % (url, add_headers, allowed_codes))
        try:
            request = urllib.request.Request(url, headers=add_headers)
            response = urllib.request.urlopen(request, timeout=5)
            data = response.read()
        except urllib.request.HTTPError as e:
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

        try:
            result = self.handle_number(args, args.number)

            # result in json format, if not found: empty fields
            j = json.dumps(result, ensure_ascii=False)
            sys.stdout.write(j)
            sys.stdout.write("\n")  # must be separate line, to avoid conversion of json into ascii
        except socket.timeout as ex:
            self.log.error("socket timeout (%s)" % ex)
            sys.exit(-1)

        # no error occurred
        sys.exit(0)
