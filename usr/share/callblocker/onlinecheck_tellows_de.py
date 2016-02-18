#!/usr/bin/env python

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2015 Patrick Ammann <pammann@gmx.net>
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
import os, sys, argparse
import urllib2
from BeautifulSoup import BeautifulSoup
import json


g_debug = False


def error(*objs):
  print("ERROR: ", *objs, file=sys.stderr)
  sys.exit(-1)

def debug(*objs):
  if g_debug: print("DEBUG: ", *objs, file=sys.stdout)
  return

def fetch_url(url):
  debug("fetch_url: " + str(url))
  data = urllib2.urlopen(url, timeout = 5)
  return data.read()


#
# main
#
def main(argv):
  global g_debug
  parser = argparse.ArgumentParser(description="Online spam check via tellows.de")
  parser.add_argument("--number", help="number to be checked", required=True)
  parser.add_argument("--username", help="partner name", required=True)
  parser.add_argument("--password", help="api key", required=True)
  parser.add_argument("--spamscore", help="score limit to mark as spam [0..9]", default=7)
  parser.add_argument('--debug', action='store_true')
  args = parser.parse_args()
  g_debug = args.debug

  # make correct format for number
  number = ""
  if args.number.startswith("+"):
    number = "00" + args.number[1:]
  else:
    error("Invalid number: " + args.number)

  url = "http://www.tellows.de/basic/num/%s?xml=1&partner=%s&apikey=%s" % (number, args.username, args.password)
  content = fetch_url(url)
  #debug(content)
  soup = BeautifulSoup(content)
  debug(soup)

  score = 5 # = neutral score
  scorelist = soup.findAll("score")
  if len(scorelist) > 0:
    score = int(scorelist[0].contents[0])

  callerName = ""
  callerTypes = soup.findAll("callertypes")
  if len(callerTypes) > 0 and len(callerTypes[0].caller.contents) > 0:
    callerName = callerTypes[0].caller.contents[0].contents[0]
  
  # result in json format
  result = {
    "spam"  : False if score < args.spamscore else True,
    "score" : score,
    "name"  : callerName
  }
  j = json.dumps(result, encoding="utf-8", ensure_ascii=False)
  sys.stdout.write(j.encode("utf8"))
  sys.stdout.write("\n") # must be seperate line, to avoid conversion of json into ascii

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

