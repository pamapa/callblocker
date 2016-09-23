#!/usr/bin/python

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

import os, sys, json, re
import subprocess
from datetime import datetime


# logging via journald
CALLBLOCKER_CALLLOGCMD       = ["journalctl", "_SYSTEMD_UNIT=callblockerd.service", "--priority", "5..5", "--since", "-12 months", "--output", "json"]
CALLBLOCKER_JOURNALALL       = ["journalctl", "_SYSTEMD_UNIT=callblockerd.service", "--lines", "1000", "--output", "json"]
CALLBLOCKER_JOURNALERRORWARN = ["journalctl", "_SYSTEMD_UNIT=callblockerd.service", "--priority", "0..4", "--lines", "1000", "--output", "json"]


def handle_journal(environ, start_response, params):
  if "all" in params: cmd = CALLBLOCKER_JOURNALALL
  else: cmd = CALLBLOCKER_JOURNALERRORWARN
  p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = p.communicate()
  #print >> sys.stderr, 'err="%s"\n' % err
  all = out.splitlines()
  all_count = len(all)
                               
  # handle paging
  start = int(params.get("start", "0"))
  count = int(params.get("count", str(all_count)))

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

  items = []  
  for i in range(start, all_count):
    if i >= start + count: break
    entry = all[all_count - i - 1] # newest first
    try:
      jj = json.loads(entry)
      prio = int(jj["PRIORITY"])
      tmp = {
        # // -6: usec -> sec => UTC time (substr because timestamp is too big for integer under 32bit
        "DATE": datetime.utcfromtimestamp(int(jj["__REALTIME_TIMESTAMP"][0:-6])).strftime("%Y-%m-%d %H:%M:%S +0000"),
        "PRIO_ID": prio,
        "PRIORITY": mapPriorityToName(prio),
        "MESSAGE": jj["MESSAGE"]
      }
      items.append(tmp)
    except ValueError:
      pass

  headers = [
    ('Content-Type',  'text/plain'),
    ('Content-Range', 'items %d-%d/%d' % (start, start+count, all_count))
  ]
  start_response('200 OK', headers)
  return [json.dumps({"numRows": all_count, "items": items})]


def handle_callerlog(environ, start_response, params):
  p = subprocess.Popen(CALLBLOCKER_CALLLOGCMD,
                       stdout=subprocess.PIPE, stderr=subprocess.PIPE)
  out, err = p.communicate()
  #print >> sys.stderr, 'err="%s"\n' % err
  all = out.splitlines()
  all_count = len(all)

  # handle paging
  start = int(params.get("start", "0"))
  count = int(params.get("count", str(all_count)))

  re_eq = r"'([^'\\]*(?:\\.[^'\\]*)*)'" # escaped quotes
  pattern = re.compile(r"^Incoming call: (number="+re_eq+")?\s?(name="+re_eq+")?\s?" +
                        "(blocked)?\s?" +
                        "(invalid)?\s?" +
                        "(whitelist="+re_eq+")?\s?(blacklist="+re_eq+")?\s?(score=([0-9]*))?$")
  items = []  
  for i in range(start, all_count):
    if i >= start + count: break
    entry = all[all_count - i - 1] # newest first
    try:
      jj = json.loads(entry)
      obj = pattern.match(jj["MESSAGE"])
      if obj:
        tmp = {
          "NUMBER": obj.group(2).strip(),
          # // -6: usec -> sec => UTC time (substr because timestamp is too big for integer under 32bit
          "DATE": datetime.utcfromtimestamp(int(jj["__REALTIME_TIMESTAMP"][0:-6])).strftime("%Y-%m-%d %H:%M:%S +0000")
        }
        try: tmp["NAME"] = obj.group(4).strip()
        except (IndexError, AttributeError): tmp["NAME"] = ""

        tmp["WHAT"] = 0;
        try:
          obj.group(5).strip()
          tmp["WHAT"] = -1 # blocked
        except (IndexError, AttributeError): pass
        try:
          obj.group(8).strip()
          tmp["WHAT"] = 1  # whitelisted
        except (IndexError, AttributeError): pass

        # human readable reason
        tmp["REASON"] = ""
        try:
          obj.group(6).strip() # invalid
          tmp["REASON"] = "invalid Caller ID"
        except (IndexError, AttributeError): pass
        try:
          add = "whitelisted in '" + obj.group(8).strip() + "'" # whitelist
          if len(tmp["REASON"]) == 0: tmp["REASON"] += add
          else: tmp["REASON"] += ", " + add
        except (IndexError, AttributeError): pass
        try:
          add = "blacklisted in '" + obj.group(10).strip() + "'" # blacklist
          if len(tmp["REASON"]) == 0: tmp["REASON"] += add
          else: tmp["REASON"] += ", " + add
        except (IndexError, AttributeError): pass
        try:
          add = "with score '" + obj.group(12).strip() + "'" # score
          if len(tmp["REASON"]) == 0: tmp["REASON"] += add
          else: tmp["REASON"] += " " + add
        except (IndexError, AttributeError): pass

        items.append(tmp)
    except ValueError:
      pass

  headers = [
    ('Content-Type',  'text/json'),
    ('Content-Range', 'items %d-%d/%d' % (start, start+count, all_count))
  ]
  start_response('200 OK', headers)
  return [json.dumps({"numRows": all_count, "items": items})]

