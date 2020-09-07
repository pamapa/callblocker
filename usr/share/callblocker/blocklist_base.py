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

import sys, argparse
import logging
import urllib.request
import json


NAME_MAX_LENGTH = 200


class BlocklistBase(object):

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
        headers = {"User-Agent": "Mozilla/5.0"}
        req = urllib.request.Request(url, headers=headers)
        data = urllib.request.urlopen(req, timeout=60)
        ret = data.read()
        ret = ret.decode("utf-8", "ignore")
        return str(ret)

    def minimize_name(self, name):
        if len(name) <= NAME_MAX_LENGTH:
            return name
        return name[0:NAME_MAX_LENGTH - 3] + "..."

    def preserve_dates(self, new_entries, old_entries):
        for n in new_entries:
            for o in old_entries:
                if n["number"] == o["number"]:
                    # was already in the old list
                    n["date_created"] = o["date_created"]
                    if n["name"] == o["name"]:
                        # did not change at all
                        n["date_modified"] = o["date_modified"]
                        #self.log.debug("found")
                    break
        return new_entries

    # remove duplicates
    # remove too small numbers -> dangerous
    # make sure numbers are in international format (e.g. +41AAAABBBBBB)
    def cleanup_entries(self, arr, country_code="+41"):
        self.log.debug("cleanup_entries (num=%s)" % len(arr))
        seen = set()
        uniq = []
        for r in arr:
            x = r["number"]

            # make international format
            if x.startswith("00"):  x = "+" + x[2:]
            elif x.startswith("0"): x = country_code + x[1:]
            else: x = country_code + x
            r["number"] = x

            # filter
            if len(x) < 5:
                # too dangerous
                self.log.debug("Skip too short number: " + str(r))
                continue
            if not x.startswith("+"):
                # not in international format
                self.log.debug("Skip unknown format number: " + str(r))
                continue
            if len(x) > 16:
                # see spec E.164 for international numbers: 15 (including country code) + 1 ("+")
                self.log.debug("Skip too long number:" + str(r))
                continue

            # filter duplicates
            if x not in seen:
                uniq.append(r)
                seen.add(x)
            else:
                self.log.debug("Skip duplicate number:" + str(r))
        self.log.debug("cleanup_entries done (num=%s)" % len(uniq))
        return uniq

    # must be implemented in the inherited class
    def get_result(self, args, last_update):
        return {}

    def run(self, args, json_filename):
        if args.debug: self.log.setLevel(logging.DEBUG)

        last_update = ""
        old_json = None
        try:
            with open(json_filename, "r") as f:
                data = f.read()
            old_json = json.loads(data)
            last_update = old_json["last_update"]
            if last_update: self.log.debug("old last_update: %s" % last_update)
        except (IOError, KeyError):
            pass

        result = self.get_result(args, last_update)

        if old_json is not None and "entries" in old_json:
            result["entries"] = self.preserve_dates(result["entries"], old_json["entries"])

        with open(json_filename, "w", encoding="utf-8") as f:
            json.dump(result, f, indent=2, ensure_ascii=False)

        # no error occurred
        sys.exit(0)
