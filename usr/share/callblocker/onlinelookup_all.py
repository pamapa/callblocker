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
import json

import onlinelookup_tel_search_ch
import onlinelookup_dasschnelle_at
import onlinelookup_dasoertliche_de


g_debug = False


def error(*objs):
  print("ERROR: ", *objs, file=sys.stderr)
  sys.exit(-1)

def debug(*objs):
  if g_debug: print("DEBUG: ", *objs, file=sys.stdout)
  return

#
# main
#
def main(argv):
  global g_debug
  parser = argparse.ArgumentParser(description="Online lookup all")
  parser.add_argument("--number", help="number to be checked", required=True)
  parser.add_argument('--debug', action='store_true')
  args = parser.parse_args()
  g_debug = args.debug

  callerName = ""
  if args.number.startswith("+41"):
    callerName = onlinelookup_tel_search_ch.lookup_number(args.number)
  elif args.number.startswith("+43"):
    callerName = onlinelookup_dasschnelle_at.lookup_number(args.number)
  elif args.number.startswith("+49"):
    callerName = onlinelookup_dasoertliche_de.lookup_number(args.number)

  # result in json format, if not found empty field
  result = {
    "name"  : callerName
  }
  j = json.dumps(result, encoding="utf-8", ensure_ascii=False)
  sys.stdout.write(j.encode("utf8"))
  sys.stdout.write("\n") # must be seperate line, to avoid conversion of json into ascii

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

