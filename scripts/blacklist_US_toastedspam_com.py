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
import re
from BeautifulSoup import BeautifulSoup
import urllib2
from collections import OrderedDict
import datetime
import demjson


def error(*objs):
  print("ERROR: ", *objs, file=sys.stderr)
  sys.exit(-1)

def debug(*objs):
  #print("DEBUG: ", *objs, file=sys.stdout)
  return

def extract_number(data):
  n = re.sub(r"[^0-9\+]","", data)
  return n

def extract_numbers(data):
  ret = []
  #print "data:" + data

  # 514-931-2572x110 -> 514-931-2572
  x = data.find("x")
  if x != -1: data = data[0:x]

  a = extract_number(data)
  if (a != ""): ret.append(a)

  return ret

def extract_comment(data):
  s = data
  if s.startswith("- "): s = s[2:]
  s = s.replace("  ", " ")
  s = s.strip()
  return s if len(s)<= 100 else s[0:100-3]+"..."

def fetch_page(url):
  debug("fetch_page: " + str(url))
  page = urllib2.urlopen(url, timeout=5)
  return page.read()

def parse_page(content):
  ret = []
  soup = BeautifulSoup(content)
  list = soup.findAll("b")
  for e in list:
    numbers = extract_numbers(e.contents[0])
    comment = extract_comment(e.nextSibling)
    for n in numbers:
      ret.append({"number":n, "comment":comment})

  return ret

# remove duplicates
# remove too small numbers -> dangerous
# make sure numbers are in international format (e.g. +41AAAABBBBBB)
def cleanup_entries(arr):
  seen = set()
  uniq = []
  for r in arr:
    x = r["number"]

    # make international format
    if not x.startswith("+"):  x = "+1"+x
    r["number"] = x

    # filter
    if len(x) < 4:
      # too dangerous
      debug("Skip too small number: " + str(r))
      continue
    if len(x) > 16:
      # see spec E.164 for international numbers: 15 (including country code) + 1 ("+")
      debug("Skip too long number:" + str(r))
      continue;

    if x not in seen:
      uniq.append(r)
      seen.add(x)
  return uniq

#
# main
#
def main(argv):
  parser = argparse.ArgumentParser(description="Fetch toastedspam blacklist")
  parser.add_argument("--output", help="output file", default="blacklist_US_toastedspam_com.json")
  args = parser.parse_args()

  content = fetch_page("http://www.toastedspam.com/phonelist.cgi")
  #debug(content)
  result = parse_page(content)
  result = cleanup_entries(result)
  if len(result) != 0:
    data = OrderedDict((
      ("name","blacklist provided by toastedspam.com"),
      ("origin", "http://www.toastedspam.com/phonelist.cgi"),
      ("parsed_by","callblocker script: "+os.path.basename(__file__)),
      ("num_entries",len(result)),
      ("last_update",datetime.datetime.now().strftime("%F %T")),
      ("entries",result)
    ))
    demjson.encode_to_file(args.output, data, overwrite=True, compactly=False, sort_keys=demjson.SORT_PRESERVE)

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

