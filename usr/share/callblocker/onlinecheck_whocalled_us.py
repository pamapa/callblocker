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
import os, sys, argparse, re
import urllib, urllib2
import demjson


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
  parser = argparse.ArgumentParser(description="Online spam check via whocalled.us")
  parser.add_argument("--number", help="number to be checked", required=True)
  parser.add_argument("--username", help="username", required=True)
  parser.add_argument("--password", help="password", required=True)
  parser.add_argument("--spamscore", help="score limit to mark as spam [-1..?]", default=5)
  args = parser.parse_args()

  url = "http://whocalled.us/do?action=getScore&name="+args.username+"&pass="+args.password+"&"+urllib.urlencode({"phoneNumber":args.number})
  content = fetch_url(url)
  debug(content)

  matchObj = re.match(r".*success=([0-9]*)[^0-9]*", content)
  if not matchObj:
    error("unexpected result: "+content)
  if int(matchObj.group(1)) != 1:
    error("not successful, result: "+content)

  score = 0
  matchObj = re.match(r".*score=([0-9]*)[^0-9]*", content)
  if matchObj:
    score = int(matchObj.group(1))
  
  # result in json format
  # caller name is not available in received content
  result = {
    "spam"  : "%s" % "false" if score < args.spamscore else "true",
    "score" : score
  }
  json = demjson.encode(result, escape_unicode=True)
  sys.stdout.write(json+'\n')

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

