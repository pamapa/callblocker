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
import sys, re
import urllib

from online_base import OnlineBase


class OnlineCheckWhoCalledUS(OnlineBase):
    def supported_country_codes(self):
        return ["+1"]

    def handle_number(self, args, number):
        url = "https://whocalled.us/do?action=getScore&%s" % urllib.urlencode({"phoneNumber": number})
        if args.username and args.password:
            url += "&name=%s&pass=%s" % (args.username, args.password)
        content = self.http_get(url)
        self.log.debug(content)

        matchObj = re.match(r".*success=([0-9]*)[^0-9]*", content)
        if not matchObj:
            self.log.error("unexpected result: " + content)
            sys.exit(-1)
        if int(matchObj.group(1)) != 1:
            self.log.error("not successful, result: " + content)
            sys.exit(-1)

        score = 0
        matchObj = re.match(r".*score=([0-9]*)[^0-9]*", content)
        if matchObj:
            score = int(matchObj.group(1))

        spam = False if score < args.spamscore else True
        return self.onlinecheck_2_result(spam, score)


#
# main
#
if __name__ == "__main__":
    m = OnlineCheckWhoCalledUS()
    parser = m.get_parser("Online check via whocalled.us")
    parser.add_argument("--username", help="username", required=False)
    parser.add_argument("--password", help="password", required=False)
    parser.add_argument("--spamscore", help="score limit to mark as spam [-1..?]", default=5)
    args = parser.parse_args()
    m.run(args)
