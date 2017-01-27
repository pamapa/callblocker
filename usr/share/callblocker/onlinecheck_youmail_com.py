#!/usr/bin/env python

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2017-2017 Patrick Ammann <pammann@gmx.net>
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
import json

from online_base import OnlineBase


class OnlineCheckYouMailCOM(OnlineBase):
    def supported_country_codes(self):
        return ["+1"]

    def handle_number(self, args, number):
        number = number[2:]  # make number local
        url = "https://dataapi.youmail.com/api/v2/phone/%s?format=json" % number
        headers = {
          "User-Agent": "Private User",
          "DataApiKey": args.username,
          "DataApiSid": args.password
        }
        content = self.http_get(url, add_headers=headers)

        data = json.loads(content)
        self.log.debug(data)

        score = 0  # = no spam
        if "spamRisk" in data and "level" in data["spamRisk"]:
          score = int(data["spamRisk"]["level"])

        spam = False if score < args.spamscore else True
        return self.onlinecheck_2_result(spam, score)


#
# main
#
if __name__ == "__main__":
    m = OnlineCheckYouMailCOM()
    parser = m.get_parser("Online check via data.youmail.com")
    parser.add_argument("--username", help="api key", required=True)
    parser.add_argument("--password", help="api sid", required=True)
    parser.add_argument("--spamscore", help="score limit to mark as spam [1..2]", default=2)
    args = parser.parse_args()
    m.run(args)
