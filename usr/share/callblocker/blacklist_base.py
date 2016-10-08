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
import sys, argparse
import logging
import urllib2
import json


NAME_MAX_LENGTH = 200


class BlacklistBase(object):

    def __init__(self):
        logging.basicConfig()
        self.log = logging.getLogger()
        self.log.setLevel(logging.WARN)

    def get_parser(self, description):
        parser = argparse.ArgumentParser(description=description)
        parser.add_argument("--output", help="output path", default=".")
        parser.add_argument('--debug', action='store_true')
        return parser

    def http_get(self, url):
        self.log.debug("http_get: '%s'" % url)
        data = urllib2.urlopen(url, timeout=5)
        return data.read()

    def minimize_name(self, name):
        if len(name) <= NAME_MAX_LENGTH:
            return name
        return name[0:NAME_MAX_LENGTH - 3] + "..."

    def preserve_dates(self, result, old_result):
        for i in range(0, len(result)):
            n = result[i]["number"]
            for o in old_result:
                if n == o["number"]:
                    # was already in the list
                    result[i]["date_created"] = o["date_created"]
                    if result[i]["name"] == o["name"]:
                        # did not change at all
                        result[i]["date_modified"] = o["date_modified"]
                        #self.log.debug("found")
                    break
        return result

    # must be implemented in the inherited class
    def get_result(self, args, last_update):
        return {}

    def run(self, args, json_filename):
        if (args.debug): self.log.setLevel(logging.DEBUG)

        last_update = ""
        json_data = None
        try:
            data = open(json_filename, "r").read()
            json_data = json.loads(data)
            last_update = json_data["last_update"]
            if last_update: self.log.debug("old last_update: %s" % last_update)
        except (IOError, KeyError):
            pass

        result = self.get_result(args, last_update)

        if json_data is not None and "entries" in json_data:
            result["entries"] = self.preserve_dates(result["entries"], json_data["entries"])

        with open(json_filename, 'w') as outfile:
            json.dump(result, outfile, indent=2)

        # no error occurred
        sys.exit(0)
