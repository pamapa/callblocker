#!/usr/bin/env python3

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2025 Patrick Ammann <pammann@gmx.net>
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

import urllib.parse
from bs4 import BeautifulSoup

from online_base import OnlineBase


class OnlineLookupDasOertlicheDE(OnlineBase):
    def supported_country_codes(self):
        return ["+49"]

    def handle_number(self, args, number: str):
        url = "https://www.dasoertliche.de/rueckwaertssuche/?" + urllib.parse.urlencode({"ph": number})
        content = self.http_get(url)

        #self.log.debug(content)
        soup = BeautifulSoup(content, "lxml")
        #self.log.debug(soup)

        caller_names = []
        addresses = soup.find_all("div", {"class": "addressblock"})
        if len(addresses) == 0:
            self.log.error("addressblock not found")
        for address in addresses:
            div_name = address.find("div", {"class": "name"})
            if not div_name:
                self.log.error("name not found")
                continue

            name = div_name.h1.contents[0]
            name = name.strip()
            caller_names.append(name)

        caller_name = "; ".join(caller_names)
        return self.onlinelookup_2_result(caller_name)


#
# main
#
if __name__ == "__main__":
    m = OnlineLookupDasOertlicheDE()
    parser = m.get_parser("Online lookup via www.dasoertliche.de")
    args = parser.parse_args()
    m.run(args)
