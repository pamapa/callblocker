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
import os
import sys
import argparse
import urllib2
import re


def error(*objs):
  print("ERROR: ", *objs, file=sys.stderr)
  sys.exit(-1)

def debug(*objs):
  #print("DEBUG: ", *objs, file=sys.stdout)
  return

def fetch_url(url):
  debug("fetch_url: " + str(url))
  data = urllib2.urlopen(url, timeout = 5)
  return data.read()


#
# main
#
def main(argv):
  parser = argparse.ArgumentParser(description="Online check via tellows.de")
  parser.add_argument("--number", help="number to be checked", required=True)
  parser.add_argument("--username", help="username", required=True)
  parser.add_argument("--password", help="password", required=True)
  parser.add_argument("--spamscore", help="spam score limit", default=5)
  args = parser.parse_args()

  url = "http://whocalled.us/do?action=getScore&name="+args.username+"&pass="+args.password+"&phoneNumber="+args.number
  content = fetch_url(url)
  debug(content)

  score = 0
  matchObj = re.match(r".*score=([0-9]*)[^0-9]*", content)#, re.M|re.I)
  if matchObj:
    score = int(matchObj.group(1))
  
  # result in json format
  print('{"spam": %s, "comment" : "whocalled.us score %s"}' % ("false" if score < args.spamscore else "true", score))

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

