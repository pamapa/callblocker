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

import sys
from bs4 import BeautifulSoup
from online_base import OnlineBase

class OnlineCheckNomorobo(OnlineBase):
    def supported_country_codes(self):
        return ["+1"]

    def handle_number(self, args, number):
        number = '{}-{}-{}'.format(number[2:5], number[5:8], number[8:]) # make number local for US
        url = "https://www.nomorobo.com/lookup/%s" % number
        headers={}
        allowed_codes=[404] # allow not found response
        content = self.http_get(url, headers, allowed_codes )
        soup = BeautifulSoup(content, "lxml")
        self.log.debug(soup)

        score = 0  # = no spam
        positions = soup.findAll(class_="profile-position")
        if len(positions) > 0:
            position = positions[0].get_text()
            if position.upper().find("DO NOT ANSWER") > -1:
                score = 2 # = is spam
            else:
                score = 1 # = might be spam (caller is "Political", "Charity", or "Debt Collector")

        caller_name = ""
        titles = soup.findAll(class_="profile-title")
        if len(titles) > 0:
            caller_name = titles[0].get_text()
            caller_name = caller_name.replace("\n", "").strip(" ")
            # TODO: if score == 1, check for "Political", "Charity", and/or "Debt Collector" 
            # in the caller_name and adjust the score if appropriate 

        spam = False if score < args.spamscore else True
        return self.onlinecheck_2_result(spam, score, caller_name)

#
# main
#
if __name__ == "__main__":
    m = OnlineCheckNomorobo()
    parser = m.get_parser("Online check via www.nomorobo.com")
    parser.add_argument("--spamscore", help="score limit to mark as spam [1..2]", default=2)
    args = parser.parse_args()
    m.run(args)
