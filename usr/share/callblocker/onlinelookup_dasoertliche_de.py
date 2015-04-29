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
import urllib, urllib2


def error(*objs):
  print("ERROR: ", *objs, file=sys.stderr)
  sys.exit(-1)

def debug(*objs):
  #print("DEBUG: ", *objs, file=sys.stdout)
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
  url = "http://www.dasoertliche.de/?form_name=search_inv&"+urllib.urlencode({"ph":number})
  content = fetch_url(url)
  #debug(content)

  callerName = ""
  # na: "Caller name"
  s = content.find("na:")
  if s != -1:
    e = content.find("\n", s)
    if s != -1:
      name = content[s+3:e].strip()
      callerName = name[1:-1]
  return callerName

#
# main
#
def main(argv):
  parser = argparse.ArgumentParser(description="Online lookup via tel.search.ch")
  parser.add_argument("--number", help="number to be checked", required=True)
  args = parser.parse_args()

  # map number to correct URL
  if not args.number.startswith("+49"):
    error("Not a valid Germany number: " + args.number)

  callerName = lookup_number(args.number)

  # result in json format, if not found empty field
  print(u'{"name": "%s"}' % (callerName))

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

