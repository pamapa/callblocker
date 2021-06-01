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

from bs4 import BeautifulSoup

from online_base import OnlineBase


class OnlineLookupPagesJaunesFR(OnlineBase):
    def supported_country_codes(self):
        return ["+33"]

    def handle_number(self, args, number):
        number = "0" + number[3:] # make number local
        url = "https://www.pagesjaunes.fr/annuaireinverse/recherche?quoiqui=%s" % number
        content = self.http_get(url, add_headers={'User-Agent': 'Mozilla'})

        #self.log.debug(content)
        soup = BeautifulSoup(content, "lxml")
        #self.log.debug(soup)

        caller_names = []
        entries = soup.findAll("li", {"class": "blocs"})
        for entry in entries:
            #self.log.debug(entry)
            title = entry.find("h3", {"class": "company-name"})
            if not title:
                self.log.error("li without h3 found")
                continue

            name = title.a.contents[0]
            name = name.strip().title()
            caller_names.append(name)

        caller_name = "; ".join(caller_names)
        return self.onlinelookup_2_result(caller_name)


#
# main
#
if __name__ == "__main__":
    m = OnlineLookupPagesJaunesFR()
    parser = m.get_parser("Online lookup via www.pagesjaunes.fr")
    args = parser.parse_args()
    m.run(args)
