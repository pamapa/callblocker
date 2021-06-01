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

from online_base import OnlineBase


class OnlineCheckTellowsDE(OnlineBase):
    def supported_country_codes(self):
        return ["+1", "+33", "+44", "+61", "+64"]

    def handle_number(self, args, number):
        # map number to correct URL
        if args.number.startswith("+1"):  # USA, Canada
            site = "www.phonespamfilter.com"
            number = number[2:]
        elif args.number.startswith("+33"):  # France
            site = "www.phonespamfilter.fr"
            number = number[3:]
        elif args.number.startswith("+44"):  # United Kingdom
            site = "www.phonespamfilter.co.uk"
            number = number[3:]
        elif args.number.startswith("+61"):  # Australia
            site = "au.phonespamfilter.com"
            number = number[3:]
        elif args.number.startswith("+64"):  # New Zealand
            site = "www.phonespamfilter.co.nz"
            number = number[3:]
        else:
            self.log.error("number '%s' is not supported '%s'" % (args.number, self.supported_country_codes()))
            sys.exit(-1)

        url = "http://%s/check.php?phone=%s" % (site, number)
        content = self.http_get(url)
        self.log.debug(content)

        score = int(content)
        spam = False if score < args.spamscore else True
        return self.onlinecheck_2_result(spam, score)


#
# main
#
if __name__ == "__main__":
    m = OnlineCheckTellowsDE()
    parser = m.get_parser("Online check via phonespamfilter.com")
    parser.add_argument("--spamscore", help="score limit to mark as spam [0..100]", default=50)
    args = parser.parse_args()
    m.run(args)
