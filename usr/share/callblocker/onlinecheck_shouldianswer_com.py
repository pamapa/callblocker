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

import urllib.parse

from bs4 import BeautifulSoup

from online_base import OnlineBase


class OnlineCheckShouldIAnswerCom(OnlineBase):
    def supported_country_codes(self):
        return ["+1", "+7", "+27", "+31", "+32", "+33", "+34", "+36", "+39", "+41", "+43", "+44", "+45", "+46", "+47", "+48", "+49", "+51", "+52", "+54", "+55", "+56", "+57", "+58", "+61", "+62", "+64", "+81", "+82", "+86", "+91", "+225", "+234", "+243", "+254", "+258", "+351", "+358", "+420", "+421", "+852"]

    def handle_number(self, args, number):
        scoreMapper = {'unknown':-1, 'positive':0, 'neutral':1, 'negative':2}
        score = -1 # = no spam
        name = ""
        
        url = "https://www.shouldianswer.com/search?" + urllib.parse.urlencode({"q": number})
        content = self.http_get(url)
        soup = BeautifulSoup(content, "lxml")

        mainInfo = soup.find("div", {"class" : "mainInfo"})
        if not mainInfo:
            self.log.error("mainInfo not found")
        else:
            scoreContainer = mainInfo.find("div", {"class" : "score"})
            if not scoreContainer:
                self.log.error("score not found in mainInfo")
            
            for c in scoreContainer.attrs['class']:
                if c == 'score':
                    continue
                score = scoreMapper.get(c)
            
            if score >= 0:
                name = mainInfo.findAll("span")[0].text.strip()

        spam = False if score < args.spamscore else True
        return self.onlinecheck_2_result(spam, score, name)

#
# main
#
if __name__ == "__main__":
    m = OnlineCheckShouldIAnswerCom()
    parser = m.get_parser("Online check via shouldianswer.com")
    parser.add_argument("--spamscore", help="score limit to mark as spam [-1..2]", default=2)
    args = parser.parse_args()
    m.run(args)
