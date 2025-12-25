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

import re
import vobject
import datetime

from import_base import ImportBase


class ImportVCF(ImportBase):
    def _extract_number(self, data):
        n = re.sub(r"[^0-9\+]", "", data)
        return n

    def _get_entity_person(self, card):
        fn = card.fn.value if card.fn else ""
        return fn

    def _parse_vcard(self, filename):
        with open(filename, "r") as f:
            data = f.read()
            f.close()

        entries = []
        now = datetime.datetime.now(datetime.UTC).strftime("%Y-%m-%d %H:%M:%S +0000")
        for card in list(vobject.readComponents(data)):
            c = card.contents
            name = self._get_entity_person(card)
            self.log.debug("Name: %s" % name)

            if "tel" in c:
                for cl in c["tel"]:
                    number = self._extract_number(cl.value)
                    field_name = cl.params["TYPE"][0].lower()
                    if field_name == "cell":
                        field_name = "mobile"
                    field_name += " phone"
                    field_name = field_name.title()
                    entries.append({"number": number, "name": name + " (" + field_name + ")",
                                   "date_created": now, "date_modified": now})
        return entries

    def get_entries(self, args):
        entries = self._parse_vcard(args.input)
        return entries


# main
#
if __name__ == "__main__":
    m = ImportVCF()
    parser = m.get_parser("Convert VCF file to json")
    args = parser.parse_args()
    m.run(args)
