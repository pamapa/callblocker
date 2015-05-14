#!/usr/bin/perl -w

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
import vobject
from collections import OrderedDict
from datetime import datetime
import demjson


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

def getEntityPerson(card):
  fn = card.fn.value if card.fn else ""
  return fn

def parse_vcard(result, filename):
  file = open(filename, "r")
  data = file.read()
  file.close()

  date = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S +0000")

  for card in list(vobject.readComponents(data)):
    c = card.contents
    name = getEntityPerson(card)
    #print("Name: %s" % name)

    if c.has_key('tel'):
      for cl in c['tel']:
        number = extract_number(cl.value)
        field_name = cl.params['TYPE'][0].lower()
        if field_name == "cell":
          field_name = "mobile2"
        field_name += " phone"
        field_name = field_name.title()
        result.append({"number":number, "name":name+" ("+field_name+")", "date_created":date, "date_modified":date})
  return result


# remove duplicates
# remove too small numbers -> dangerous
# make sure numbers are in international format (e.g. +41AAAABBBBBB)
def cleanup_entries(arr, country_code):
  debug("cleanup_entries...")
  seen = set()
  uniq = []
  for r in arr:
    x = r["number"]

    # make international format
    if x.startswith("00"):  x = "+"+x[2:]
    elif x.startswith("0"): x = country_code+x[1:]
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

    if x not in seen:
      uniq.append(r)
      seen.add(x)

  debug("cleanup_entries done")
  return uniq

#
# main
#
def main(argv):
  global result, g_debug
  parser = argparse.ArgumentParser(description="Convert LDIF file to json")
  parser.add_argument("--input", help="input file", required=True)
  parser.add_argument("--country_code", help="country code, e.g. +41", required=True)
  parser.add_argument("--merge", help="file to merge with", default="out.json")
  args = parser.parse_args()
  g_debug = args.debug

  name = os.path.splitext(os.path.basename(args.input))[0]
  result = []

  # merge
  try:
    data = open(args.merge, "r").read()
    json = demjson.decode(data)
    name = json["name"]
    result = json["entries"]
    debug(result)
  except IOError:
    pass

  # convert
  result = parse_vcard(result, args.input)

  result = cleanup_entries(result, args.country_code)
  if len(result) != 0:
    data = OrderedDict((
      ("name", name),
      ("entries", result)
    ))
    demjson.encode_to_file(args.merge,
                           data, overwrite=True, compactly=False, sort_keys=demjson.SORT_PRESERVE)

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

