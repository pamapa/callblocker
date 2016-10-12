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
import urllib
from BeautifulSoup import BeautifulSoup

from online_base import OnlineBase


class OnlineLookupPagesJaunesFR(OnlineBase):
    def supported_country_codes(self):
        return ["+33"]

    def handle_number(self, args, number):
        number = "0" + number[3:] # make number local
        url = "http://www.pagesjaunes.fr/annuaireinverse/recherche?" + urllib.urlencode({"quoiqui": number}) + "&proximite=0"
        content = self.http_get(url)
        soup = BeautifulSoup(content)
        self.log.debug(soup)

        caller_name = unicode("")
        entry = soup.find("a", {"class": "denomination-links pj-lb pj-link"})
        if entry:
            caller_name = unicode(entry.contents[0]).strip()

        return self.onlinelookup_2_result(caller_name)


#
# main
#
if __name__ == "__main__":
    m = OnlineLookupPagesJaunesFR()
    parser = m.get_parser("Online lookup via pagesjaunes.fr")
    args = parser.parse_args()
    m.run(args)
