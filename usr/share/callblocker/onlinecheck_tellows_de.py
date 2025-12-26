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

from bs4 import BeautifulSoup

from online_base import OnlineBase


class OnlineCheckTellowsDE(OnlineBase):
    def supported_country_codes(self):
        return ["+"]

    def handle_number(self, args, number: str):
        number = "00" + number[1:]
        url = "https://www.tellows.de/basic/num/%s?xml=1&partner=%s&apikey=%s" % (number, args.username, args.password)
        content = self.http_get(url)
        soup = BeautifulSoup(content, "xml")
        self.log.debug(soup)

        score = 5  # = neutral score
        score_list = soup.find_all("score")
        if len(score_list) > 0:
            score = int(score_list[0].contents[0])

        caller_name = ""
        caller_types = soup.find_all("callertypes")
        if len(caller_types) > 0 and len(caller_types[0].caller.contents) > 0:
            caller_name = caller_types[0].caller.contents[0].contents[0]

        spam = False if score < args.spamscore else True
        return self.onlinecheck_2_result(spam, score, caller_name)


#
# main
#
if __name__ == "__main__":
    m = OnlineCheckTellowsDE()
    parser = m.get_parser("Online check via www.tellows.de")
    parser.add_argument("--username", help="partner name", required=True)
    parser.add_argument("--password", help="api key", required=True)
    parser.add_argument("--spamscore", help="score limit to mark as spam [1..9]", default=7)
    args = parser.parse_args()
    m.run(args)
