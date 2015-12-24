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
from BeautifulSoup import BeautifulSoup
import urllib2
from collections import OrderedDict
from datetime import datetime
import json


NAME_MAX_LENGTH = 200
g_debug = False


def error(*objs):
  print("ERROR: ", *objs, file=sys.stderr)
  sys.exit(-1)

def debug(*objs):
  if g_debug: print("DEBUG: ", *objs, file=sys.stdout)
  return

def extract_number(data):
  n = re.sub(r"[^0-9\+]","", data)
  return n

# 021 558 73 91/92/93/94/95
def extract_slashed_numbers(data):
  ret = []
  arr = data.split("/")
  a0 = extract_number(arr[0])
  if (a0 != ""):
    ret.append(a0)
    base = a0[0:-2]
    for ax in arr[1:]:
      ax = extract_number(base + ax)
      if (ax != ""):
        ret.append(ax)
  return ret

# 044 400 00 00 bis 044 400 00 19
def extract_range_numbers(data):
  ret = []
  arr = re.split("bis", data)
  s = extract_number(arr[0])
  e = extract_number(arr[1])
  for i in range(int(s[-4:]), int(e[-4:])+1):
    a = s[:-4]+"%04d" % i
    ret.append(a)
  return ret

def extract_numbers(data):
  ret = []
  #print("data:" + data)
  arr = re.split("und|oder|sowie|auch|,|;", data)
  for a in arr:
    if a.find("/") != -1:
      ret.extend(extract_slashed_numbers(a))
    elif a.find("bis") != -1:
      ret.extend(extract_range_numbers(a))
    else:
      a = extract_number(a)
      if (a != ""): ret.append(a)
  return ret

def extract_name(data):
  s = unicode(data)
  s = s.replace("\n", "").replace("\r", "")
  s = re.sub(r'<[^>]*>', " ", s) # remove tags
  s = s.replace("&amp", "&")
  s = s.replace("  ", " ")
  s = s.strip()
  return s if len(s)<= NAME_MAX_LENGTH else s[0:NAME_MAX_LENGTH-3]+"..."

def fetch_page(page_nr):
  print("fetch_page: " + str(page_nr))
  page = urllib2.urlopen("https://www.ktipp.ch/service/warnlisten/detail/?warnliste_id=7&ajax=ajax-search-form&page=" + str(page_nr), timeout=10)
  return page.read()

def extract_str(data, start_str, end_str, error_msg):
  s = data.find(start_str)
  if (s == -1): error(error_msg+". Start ("+start_str+") not found.")
  s += len(start_str)
  e = data.find(end_str, s)
  if (e == -1): error(error_msg+". End ("+end_str+") not found.")
  return data[s:e].strip()

def parse_page(soup):
  ret = []
  debug("parse_page...")
  list = soup.findAll("section",{"class":"teaser cf"})

  date = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S +0000")

  for e in list:
    numbers = extract_numbers(e.strong.contents[0])
    name = extract_name(e.p)
    for n in numbers:
      ret.append({"number":n, "name":name, "date_created":date, "date_modified":date})
  debug("parse_page done")
  return ret

def parse_pages(content):
  ret = []

  soup = BeautifulSoup(content)
  tmp = str(soup.findAll("li")[-1])
  max_page_str = extract_str(tmp, "ajaxPagerWarnlisteLoadIndex(", ")", "Can't extract max pages")
  last_page = int(max_page_str)
  #print last_page
  
  ret.extend(parse_page(soup))
  #return ret
  for p in range(1,last_page+1):
    content = fetch_page(p)
    debug("fetch done, BeautifulSoup...")
    soup = BeautifulSoup(content)
    debug("BeautifulSoup done")
    ret.extend(parse_page(soup))
  return ret

# remove duplicates
# remove too small numbers -> dangerous
# make sure numbers are in international format (e.g. +41AAAABBBBBB)
def cleanup_entries(arr):
  debug("cleanup_entries...")
  seen = set()
  uniq = []
  for r in arr:
    x = r["number"]

    # make international format
    if x.startswith("00"):  x = "+"+x[2:]
    elif x.startswith("0"): x = "+41"+x[1:]
    r["number"] = x

    # filter
    if len(x) < 4:
      # too dangerous
      debug("Skip too small number: " + str(r))
      continue
    if not x.startswith("+"):
      # not in international format
      debug("Skip unknown format number: " + str(r))
      continue;
    if len(x) > 16:
      # see spec E.164 for international numbers: 15 (including country code) + 1 ("+")
      debug("Skip too long number:" + str(r))
      continue;

    # filter duplicates
    if x not in seen:
      uniq.append(r)
      seen.add(x)

  debug("cleanup_entries done")
  return uniq

def preserve_dates(result, older):
  for i in range(0, len(result)):
    n = result[i]["number"]
    for o in older:
      if n == o["number"]:
        # was alraedy in the list
        result[i]["date_created"] = o["date_created"]
        if result[i]["name"] == o["name"]:
          # did not change at all
          result[i]["date_modified"] = o["date_modified"]
        #debug("found")
        break
  return result


#
# main
#
def main(argv):
  global g_debug
  parser = argparse.ArgumentParser(description="Fetch blacklist provided by ktipp.ch")
  parser.add_argument("--output", help="output path", default=".")
  parser.add_argument('--debug', action='store_true')
  args = parser.parse_args()
  g_debug = args.debug
  json_filename = args.output+"/blacklist_ktipp_ch.json"

  last_update = ""
  json_data = None
  try:
    data = open(json_filename, "r").read()
    json_data = json.loads(data)
    last_update = json_data["last_update"]
    debug(last_update)
  except (IOError, KeyError):
    pass

  content = fetch_page(0)
  source_date = unicode(extract_str(content, "Letzte Aktualisierung:", "<", "Can't extract creation date"))
  debug(source_date)
  if last_update == source_date:
    # we already have this version
    debug("We already have this version")
    return

  result = parse_pages(content)
  result = cleanup_entries(result)

  if json_data is not None and "entries" in json_data:
    result = preserve_dates(result, json_data["entries"])

  if len(result) != 0:
    data = OrderedDict((
      ("name", "ktipp.ch blacklist"),
      ("origin", "https://www.ktipp.ch/service/warnlisten/detail/?warnliste_id=7"),
      ("last_update", source_date),
      ("parsed_by", "callblocker script: "+os.path.basename(__file__)),
      ("num_entries", len(result)),
      ("entries", result)
    ))
    with open(json_filename, 'w') as outfile:
      json.dump(data, outfile, indent=2)

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

