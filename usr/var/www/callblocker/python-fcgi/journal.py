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

import json, re
from datetime import datetime, timezone
from systemd import journal


def handle_journal(environ, start_response, params):
    j = journal.Reader()
    j.add_match(_SYSTEMD_UNIT="callblockerd.service")
    if not "all" in params:
        j.log_level(journal.LOG_WARNING) # set max log level

    # handle paging
    start = int(params.get("start", "0"))
    count = int(params.get("count", "25"))

    # we iterate backwards: newest first
    j.seek_tail()
    if start != 0:
        j.get_previous(start)

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
    i = start
    for i in range(start, start + count):
        entry = j.get_previous()
        if len(entry) == 0: break
        date = entry["__REALTIME_TIMESTAMP"]
        # patch: we need tzinfo
        tz_local = datetime.now(timezone.utc).astimezone().tzinfo
        date = date.replace(tzinfo=tz_local)

        tmp = {
            "date": date.strftime("%Y-%m-%d %H:%M:%S %z"),
            "prio_id": entry["PRIORITY"],
            "priority": mapPriorityToName(entry["PRIORITY"]),
            "message": entry["MESSAGE"]
        }
        items.append(tmp)

    # find all_count
    while True:
        entry = j.get_previous()
        if len(entry) == 0: break
        i += 1
    all_count = i

    # prepare response
    headers = [
        ("Content-Type",  "text/plain"),
        ("Content-Range", "items %d-%d/%d" % (start, start + len(items) - 1, all_count))
    ]
    start_response("200 OK", headers)
    return [json.dumps({"numRows": all_count, "items": items})]


def handle_callerlog(environ, start_response, params):
    j = journal.Reader()
    j.add_match(_SYSTEMD_UNIT="callblockerd.service")
    j.add_match(PRIORITY="%d" % journal.LOG_NOTICE)

    # handle paging
    start = int(params.get("start", "0"))
    count = int(params.get("count", "25"))

    # we iterate backwards: newest first
    j.seek_tail()
    if start != 0:
        j.get_previous(start)

    re_eq = r"'([^'\\]*(?:\\.[^'\\]*)*)'" # escaped quotes
    pattern = re.compile(r"^Incoming call: (number="+re_eq+")?\s?(name="+re_eq+")?\s?" +
                         "(blocked)?\s?" +
                         "(invalid)?\s?" +
                         "(whitelist="+re_eq+")?\s?(blacklist="+re_eq+")?\s?(score=([0-9]*))?$")

    items = []
    i = start
    for i in range(start, start + count):
        entry = j.get_previous()
        if len(entry) == 0: break

        obj = pattern.match(entry["MESSAGE"])
        if obj:
            date = entry["__REALTIME_TIMESTAMP"]
            # patch: we need tzinfo
            tz_local = datetime.now(timezone.utc).astimezone().tzinfo
            date = date.replace(tzinfo=tz_local)

            tmp = {
                "date": date.strftime("%Y-%m-%d %H:%M:%S %z"),
                "number": obj.group(2).strip()
            }
            # name
            tmp["name"] = ""
            try: tmp["name"] = obj.group(4).strip()
            except (IndexError, AttributeError): pass
            tmp["name"] = tmp["name"].replace("\\'", "'") # unescape possible quotes
            # what
            tmp["what"] = 0;
            try:
                obj.group(5).strip()
                tmp["what"] = -1 # blocked
            except (IndexError, AttributeError): pass
            try:
                obj.group(8).strip()
                tmp["what"] = 1  # whitelisted
            except (IndexError, AttributeError): pass
            # human readable reason
            tmp["reason"] = ""
            try:
                obj.group(6).strip() # invalid
                tmp["reason"] = "invalid Caller ID"
            except (IndexError, AttributeError): pass
            try:
                add = "whitelisted in '" + obj.group(8).strip() + "'" # whitelist
                if len(tmp["reason"]) == 0: tmp["reason"] += add
                else: tmp["reason"] += ", " + add
            except (IndexError, AttributeError): pass
            try:
                add = "blacklisted in '" + obj.group(10).strip() + "'" # blacklist
                if len(tmp["reason"]) == 0: tmp["reason"] += add
                else: tmp["reason"] += ", " + add
            except (IndexError, AttributeError): pass
            try:
                add = "with score '" + obj.group(12).strip() + "'" # score
                if len(tmp["reason"]) == 0: tmp["reason"] += add
                else: tmp["reason"] += " " + add
            except (IndexError, AttributeError): pass
            # add
            items.append(tmp)

    # find all_count
    while True:
        entry = j.get_previous()
        if len(entry) == 0: break
        i += 1
    all_count = i

    # prepare response
    headers = [
        ("Content-Type",  "text/plain"),
        ("Content-Range", "items %d-%d/%d" % (start, start + len(items) - 1, all_count))
    ]
    start_response("200 OK", headers)
    return [json.dumps({"numRows": all_count, "items": items})]
