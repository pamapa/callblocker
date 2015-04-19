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
from ldif import LDIFParser
from collections import OrderedDict
from datetime import datetime
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

def getEntityPerson(entry):
  fname = ""
  sname = ""
  cname = ""
  if "givenName" in entry: fname = entry["givenName"][0].decode(encoding='UTF-8',errors='strict')
  if "sn" in entry: sname = entry["sn"][0].decode(encoding='UTF-8',errors='strict')
  if "cn" in entry: cname = entry["cn"][0].decode(encoding='UTF-8',errors='strict')
  name = ""
  if sname == "":
    name = cname
  elif fname == "":
    name = sname;
  else:
    name = fname + " " + sname
  return name

result = []
class MyLDIF(LDIFParser):
  def __init__(self, input, output):
    LDIFParser.__init__(self, input)
    self.date = datetime.utcnow().strftime("%Y-%m-%d %H:%M:%S +0000")
    #debug(self.date)
    
  def handle(self, dn, entry):
    #debug(entry)
    name = getEntityPerson(entry)
    number = ""
    field_name = ""
    if "mobile" in entry:
      number = extract_number(entry["mobile"][0])
      field_name = "Mobile Phone"
    if "homePhone" in entry:
      number = extract_number(entry["homePhone"][0])
      field_name = "Home Phone"
    if "telephoneNumber" in entry:
      number = extract_number(entry["telephoneNumber"][0])
      field_name = "Work Phone"

    if len(number) != 0:    
      result.append({"number":number, "name":name+" ("+field_name+")", "date_created":self.date, "date_modified":self.date})


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
  global result
  parser = argparse.ArgumentParser(description="Convert LDIF file to json")
  parser.add_argument("--input", help="input file", required=True)
  parser.add_argument("--country_code", help="country code, e.g. +41", required=True)
  parser.add_argument("--merge", help="file to merge with", default="out.json")
  args = parser.parse_args()

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
  parser = MyLDIF(open(args.input, "r"), sys.stdout)
  parser.parse()
  
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

