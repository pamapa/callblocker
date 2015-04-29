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
from BeautifulSoup import BeautifulSoup
import demjson


def error(*objs):
  print("ERROR: ", *objs, file=sys.stderr)
  sys.exit(-1)

def debug(*objs):
  print("DEBUG: ", *objs, file=sys.stdout)
  return

def fetch_url(url):
  debug("fetch_url: '" + str(url)+"'")
  data = urllib2.urlopen(url, timeout = 5)
  return data.read()

def extract_callerName(name):
  matchObj = re.match(r"<a.*>(.*)</a>", name)
  if matchObj: name = matchObj.group(1)
  matchObj = re.match(r"(.*)<span.*>(.*)</span>", name)
  if matchObj: name = matchObj.group(1) + matchObj.group(2)
  return name

def lookup_number(number):
  url = "http://tel.search.ch/api/?" + urllib.urlencode({"was":number})
  content = fetch_url(url)
  #debug(content)
  soup = BeautifulSoup(content)
  #debug(soup)

  callerName = ""
  entries = soup.findAll("entry")
  for entry in entries:
    name = entry.title.contents[0]
    if len(callerName) == 0:
      callerName = name
    else:
      callerName += "; " + name
  return callerName

#
# main
#
def main(argv):
  parser = argparse.ArgumentParser(description="Online lookup via tel.search.ch")
  parser.add_argument("--number", help="number to be checked", required=True)
  args = parser.parse_args()

  # map number to correct URL
  if not args.number.startswith("+41"):
    error("Not a valid Swiss number: " + args.number)

  callerName = lookup_number(args.number)

  # result in json format, if not found empty field
  json = demjson.encode({"name" : callerName}, encoding="utf-8")
  sys.stdout.write(json)
  sys.stdout.write("\n") # must be seperate line, to avoid conversion of json into ascii

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

