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

def extract_callerName(name):
  matchObj = re.match(r"<a.*>(.*)</a>", name)
  if matchObj: name = matchObj.group(1)
  matchObj = re.match(r"(.*)<span.*>(.*)</span>", name)
  if matchObj: name = matchObj.group(1) + matchObj.group(2)
  return name

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

  url = "http://tel.search.ch/index.en.html?was=" + args.number
  content = fetch_url(url)
  #debug(content)

  callerName = ""

  # private and bussiness (single entry)
  if len(callerName) == 0:
    x = content.find('class="tel-detail-avatar')
    if x != -1:
      debug('found class="tel-detail-avatar')
      h1s = content.find("<h1>", x + 1)
      if h1s != -1:
        h1s += 4
        h1e = content.find("</h1>", h1s + 1)
        callerName = extract_callerName(content[h1s:h1e])

  # private (multiple entries)
  if len(callerName) == 0:
    x = 0
    while True:
      x = content.find('class="tel-person', x)
      if x == -1: break;
      debug('found class="tel-person')
      x += 1
      h1s = content.find("<h1>", x)
      if h1s == -1: continue
      h1s += 4
      h1e = content.find("</h1>", h1s + 1)
      if h1e == -1: continue
      debug("found: "+content[h1s:h1e])
      callerName += extract_callerName(content[h1s:h1e])+"; ";
    callerName = callerName[:-2] # remove last "; "

  # bussiness
  if len(callerName) == 0:
    x = content.find('class="tel-commercial')
    if x != -1:
      debug('found class="tel-commercial')
      h1s = content.find("<h1>", x + 1)
      if h1s != -1:
        h1s += 4
        h1e = content.find("</h1>", h1s + 1)
        callerName = extract_callerName(content[h1s:h1e])
  
  debug(callerName)

  # result in json format
  print('{"name": "%s"}' % (callerName))

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

