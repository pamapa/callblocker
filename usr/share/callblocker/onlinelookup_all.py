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
import onlinelookup_tel_search_ch
import onlinelookup_dasschnelle_at


def error(*objs):
  print("ERROR: ", *objs, file=sys.stderr)
  sys.exit(-1)

def debug(*objs):
  #print("DEBUG: ", *objs, file=sys.stdout)
  return

#
# main
#
def main(argv):
  parser = argparse.ArgumentParser(description="Online lookup all")
  parser.add_argument("--number", help="number to be checked", required=True)
  args = parser.parse_args()

  callerName = ""
  if args.number.startswith("+41"):
    callerName = onlinelookup_tel_search_ch.lookup_number(args.number)
  elif args.number.startswith("+43"):
    callerName = onlinelookup_dasschnelle_at.lookup_number(args.number)

  # result in json format, if not found empty field
  print('{"name": "%s"}' % (callerName))

if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

