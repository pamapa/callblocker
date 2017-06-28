#!/usr/bin/env python3

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

import urllib.parse

from online_base import OnlineBase


class OnlineLookupDasOertlicheDE(OnlineBase):
    def supported_country_codes(self):
        return ["+49"]

    def handle_number(self, args, number):
        url = "http://www.dasoertliche.de/?form_name=search_inv&" + urllib.parse.urlencode({"ph": number})
        content = self.http_get(url)
        content = content.decode()
        #self.log.debug(content)

        caller_name = ""
        # na: "Caller name"
        s = content.find("na:")
        if s != -1:
            e = content.find("\n", s)
            if e != -1:
                name = content[s+3:e].strip()
                caller_name = name[1:-1]

        return self.onlinelookup_2_result(caller_name)


#
# main
#
if __name__ == "__main__":
    m = OnlineLookupDasOertlicheDE()
    parser = m.get_parser("Online lookup via www.dasoertliche.de")
    args = parser.parse_args()
    m.run(args)
