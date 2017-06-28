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

import os, sys, argparse
import logging
from collections import OrderedDict
import json
import codecs


class ImportBase(object):

    def __init__(self):
        logging.basicConfig()
        self.log = logging.getLogger()
        self.log.setLevel(logging.WARN)

    def get_parser(self, description):
        parser = argparse.ArgumentParser(description=description)
        parser.add_argument('--debug', action='store_true')
        parser.add_argument("--input", help="input file", required=True)
        parser.add_argument("--country_code", help="country code, e.g. +41", required=True)
        parser.add_argument("--merge", help="file to merge with", default="out.json")
        return parser

    # remove duplicates
    # remove too small numbers -> dangerous
    # make sure numbers are in international format (e.g. +41AAAABBBBBB)
    def cleanup_entries(self, arr, country_code):
        self.log.debug("cleanup_entries (num=%s)" % len(arr))
        seen = set()
        uniq = []
        for r in arr:
            x = r["number"]

            # make international format
            if x.startswith("00"):  x = "+"+x[2:]
            elif x.startswith("0"): x = country_code+x[1:]
            r["number"] = x

            # filter
            if len(x) < 4:
                # too dangerous
                self.log.debug("Skip too small number: " + str(r))
                continue
            if not x.startswith("+"):
                # not in international format
                self.log.debug("Skip unknown format number: " + str(r))
                continue
            if len(x) > 16:
                # see spec E.164 for international numbers: 15 (including country code) + 1 ("+")
                self.log.debug("Skip too long number:" + str(r))
                continue

            if x not in seen:
                uniq.append(r)
                seen.add(x)

        self.log.debug("cleanup_entries done (num=%s)" % len(uniq))
        return uniq

    def merge_entries(self, new_entries, old_entries):
        # preserve dates if possible
        for n in new_entries:
            found =  None
            for o in old_entries:
                if n["number"] == o["number"]:
                    # was already in the old list
                    n["date_created"] = o["date_created"]
                    if n["name"] == o["name"]:
                        # did not change at all
                        n["date_modified"] = o["date_modified"]
                        #self.log.debug("found")
                    found = o
                    break
            if found:
                old_entries.remove(found)

        entries = new_entries + old_entries
        return entries

    # must be implemented in the inherited class
    def get_entries(self, args):
        return []

    def run(self, args):
        if args.debug: self.log.setLevel(logging.DEBUG)

        # merge
        old_json = None
        try:
            with open(args.merge, "r") as f:
                data = f.read().decode("utf-8-sig")
            old_json = json.loads(data)
        except IOError:
            pass

        entries = self.get_entries(args)
        if old_json is not None and "entries" in old_json:
            entries = self.merge_entries(entries, old_json["entries"])
        entries = self.cleanup_entries(entries, args.country_code)

        result = OrderedDict()
        result["name"] = os.path.splitext(os.path.basename(args.merge))[0]
        result["num_entries"] = len(entries)
        result["entries"] = entries

        with codecs.open(args.merge, "w", encoding="utf-8") as f:
            json.dump(result, f, indent=2, ensure_ascii=False)

        # no error occurred
        sys.exit(0)
