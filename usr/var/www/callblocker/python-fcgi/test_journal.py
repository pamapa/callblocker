# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2017 Patrick Ammann <pammann@gmx.net>
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

import select
from datetime import datetime
from systemd import journal


if __name__ == "__main__":
  print("Testing...")

  j = journal.Reader()
  #j.log_level(journal.LOG_INFO) # maximum log level
  #j.add_match(_SYSTEMD_UNIT="init.scope")
  #j.add_match(_SYSTEMD_UNIT="callblockerd.service")

  # handle paging
  start = 0
  count = 12

  def mapPriorityToName(prio):
    if prio == 0: return "Emergency"
    if prio == 1: return "Alert"
    if prio == 2: return "Critical"
    if prio == 3: return "Error"
    if prio == 4: return "Warning"
    if prio == 5: return "Notice"
    if prio == 6: return "Information"
    if prio == 7: return "Debug"
    return "Unknown"

  # we iterate backwards: newest first
  j.seek_tail()
  if start != 0:
    j.get_previous(start)

  #i = 0
  items = []
  now = datetime.now()
  for i in range(start, count):
    print i
  #while True:
    entry = j.get_previous()
    if len(entry) == 0: break
    date = entry['__REALTIME_TIMESTAMP']
    #print date

    # since X days
    #days = (now - date).days
    #if days > 365:
    #  break

    # max X items
    #if i + 1 > 300:
    #  break

    if True: #start <= i and i < start + count:
      tmp = {
        "date": date.strftime("%Y-%m-%d %H:%M:%S +0000"),
        "prio_id": entry["PRIORITY"],
        "priority": mapPriorityToName(entry["PRIORITY"]),
        "message": entry["MESSAGE"]
      }
      items.append(tmp)
    i += 1
  #all_count = i
  all_count = "*"

  # TODO: experiment with:
  # - no real end.... -> all_count = ? -> "items 0-10/*"
  # - get_previous multiple jump -> aka start != 0

  print('Content-Range', 'items %d-%d/%s' % (start, start + len(items) - 1, all_count))
  print items


  print("done")

