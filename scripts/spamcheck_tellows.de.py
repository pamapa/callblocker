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

import os
import sys
import argparse
import urllib2
from BeautifulSoup import BeautifulSoup


def error(msg):
  print msg
  sys.exit(-1)

def debug(msg):
  #print msg
  return

def fetch_url(url):
  debug("fetch_url: " + str(url))
  data = urllib2.urlopen(url)
  return data.read()


#
# main
#
def main(argv):
  parser = argparse.ArgumentParser(description="Online check via tellows.de")
  parser.add_argument("--number", help="number to be checked", required=True)
  parser.add_argument("--username", help="partner name", required=True)
  parser.add_argument("--password", help="api key", required=True)
  args = parser.parse_args()

  number = "";
  if args.number.startswith("+"):
    number = "00" + args.number[1:]
  else:
    error("invalid number %s" + args.number)
    sys.exit(1)

  url = "http://www.tellows.de/basic/num/"+number+"?xml=1&partner="+args.username+"&apikey="+args.password;
  print url  

  content = fetch_url(url)
  #print content
  soup = BeautifulSoup(content)
  #print soup
  scorelist = soup.findAll("score")
  score = int(scorelist[0].contents[0])
  
  # result in json format
  print '{"spam: %s", comment="tellows.de score %s"}' % ("false" if score < 7 else "true", score)


if __name__ == "__main__":
    main(sys.argv)
    sys.exit(0)

