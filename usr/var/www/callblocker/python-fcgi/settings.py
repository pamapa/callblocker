# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2019 Patrick Ammann <pammann@gmx.net>
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

import os, sys, json
import subprocess
import cgi
from datetime import datetime, timezone

import config


SETTINGS_FILE = os.path.join(config.CALLBLOCKER_SYSCONFDIR, "settings.json")


def _load_settings():
    if not os.path.exists(SETTINGS_FILE):
        with open(SETTINGS_FILE, "w", encoding="utf-8") as f:
            empty = {"log_level": "info", "pjsip_log_level": 0, "phones": []}
            json.dump(empty, f, indent=2, ensure_ascii=False)
    with open(SETTINGS_FILE) as f:
        jj = json.loads(f.read())
    return jj


# fix wrong padded dates (came from old app.js/date2UTCString: e.g. 2015-4-9 10:14:3 +0000)
def _fix_date_string(date_str):
    try:
        date = datetime.strptime(date_str, "%Y-%m-%d %H:%M:%S %z")
    except:
       date = datetime.now()
    return date.strftime("%Y-%m-%d %H:%M:%S %z")


def _remove_duplicates(entries):
    uniq = []
    seen = set()
    for entry in entries:
        number = entry["number"]

        # fix wrong padded dates
        if "date_modified" in entry:
            entry["date_modified"] = _fix_date_string(entry["date_modified"])
        if "date_created" in entry:
            entry["date_created"] = _fix_date_string(entry["date_created"])

        # filter duplicates
        if number not in seen:
            uniq.append(entry)
            seen.add(number)
    return uniq


def _create_empty_list(fullname, name):
    with open(fullname, "w", encoding="utf-8") as f:
        json.dump({"name": name, "entries": []}, f, indent=2, ensure_ascii=False)


def handle_phones(environ, start_response, params):
    jj = _load_settings()

    if environ.get('REQUEST_METHOD', '') == "POST":
        post = cgi.FieldStorage(fp=environ['wsgi.input'], environ=environ, keep_blank_values=True)
        #print >> sys.stderr, 'POST data=%s\n' % post.getvalue('data')
        json_phones = json.loads(post.getvalue('data'))
        # Analog vs. SIP: remove "device" or others
        for phone in json_phones["items"]:
            if phone.get("device", "") == "":
                # mark as SIP
                phone.pop("device", None)
            else:
                # mark as Analog
                phone.pop("domain", None)
                phone.pop("username", None)
                phone.pop("password", None)
                phone.pop("realm", None)
                phone.pop("secure", None)
        jj["phones"] = json_phones["items"]
        with open(SETTINGS_FILE, "w", encoding="utf-8") as f:
            json.dump(jj, f, indent=4, ensure_ascii=False)
        return

    all = []
    if "phones" in jj:
        all = jj["phones"]
    all_count = len(all)

    # handle paging
    start = int(params.get("start", "0"))
    count = int(params.get("count", str(all_count)))

    items = []
    for i in range(start, all_count):
        if i >= start + count: break
        entry = all[i]
        items.append(entry)

    headers = [
        ('Content-Type',  'text/json'),
        ('Content-Range', 'items %d-%d/%d' % (start, start+count, all_count))
    ]
    start_response('200 OK', headers)
    return [json.dumps({"numRows": all_count, "items": items})]


def handle_online_credentials(environ, start_response, params):
    jj = _load_settings()

    if environ.get('REQUEST_METHOD', '') == "POST":
        post = cgi.FieldStorage(fp=environ['wsgi.input'], environ=environ, keep_blank_values=True)
        #print >> sys.stderr, 'POST data=%s\n' % post.getvalue('data')
        json_creds = json.loads(post.getvalue('data'))
        jj["online_credentials"] = json_creds["items"]
        with open(SETTINGS_FILE, 'w') as f:
            json.dump(jj, f, indent=4)
        return

    all = []
    if "online_credentials" in jj:
        all = jj["online_credentials"]
    all_count = len(all)

    # handle paging
    start = int(params.get("start", "0"))
    count = int(params.get("count", str(all_count)))

    items = []
    for i in range(start, all_count):
        if i >= start + count: break
        entry = all[i]
        items.append(entry)

    headers = [
        ('Content-Type',  'text/json'),
        ('Content-Range', 'items %d-%d/%d' % (start, start+count, all_count))
    ]
    start_response('200 OK', headers)
    return [json.dumps({"numRows": all_count, "items": items})]


def handle_get_list(environ, start_response, params):
    dirname = "blocklists"
    if "dirname" in params:
        dirname = params["dirname"]
        if dirname != "blocklists" and dirname != "allowlists" and dirname != "cache":
            start_response('404 NOT FOUND', [('Content-Type', 'text/plain')])
            return ['Not Found']

    filename = "main.json"
    if "filename" in params:
        filename = os.path.basename(params["filename"])
    fullname = os.path.join(config.CALLBLOCKER_SYSCONFDIR, dirname, filename)
    if not os.path.exists(fullname):
        if filename in ["main.json", "onlinelookup.json", "onlinecheck.json"]:
            # to keep app.js simple: create empty
            _create_empty_list(fullname, os.path.splitext(fullname)[0])
        else:
            start_response('404 NOT FOUND', [('Content-Type', 'text/plain')])
            return ['Not Found']

    if environ.get('REQUEST_METHOD', '') == "POST":
        post = cgi.FieldStorage(fp=environ['wsgi.input'], environ=environ, keep_blank_values=True)
        #print >> sys.stderr, 'POST data=%s\n' % post.getvalue('data')
        json_list = json.loads(post.getvalue('data'))
        with open(fullname, "w", encoding="utf-8") as f:
            json.dump({"name": json_list["label"], "entries": _remove_duplicates(json_list["items"])}, f, indent=2, ensure_ascii=False)
        return

    with open(fullname, encoding="utf-8") as f:
        jj = json.loads(f.read())
    all = _remove_duplicates(jj["entries"])
    # sort entries by name
    all = sorted(all, key=lambda k: k['name'])
    all_count = len(all)

    # handle paging
    start = int(params.get("start", "0"))
    count = int(params.get("count", str(all_count)))

    items = []
    for i in range(start, all_count):
        if i >= start + count: break
        entry = all[i]
        items.append(entry)

    # make sure we have a label/name
    if "name" in jj:
        label = jj["name"]
    else:
        label = os.path.splitext(fullname)[0]

    headers = [
        ('Content-Type',  'text/json'),
        ('Content-Range', 'items %d-%d/%d' % (start, start+count, all_count))
    ]
    start_response('200 OK', headers)
    return [json.dumps({"label": label, "identifier": "number", "numRows": all_count, "items": items})]


def handle_get_lists(environ, start_response, params):
    dirname = "blocklists"
    if "dirname" in params:
        dirname = params["dirname"]
        if dirname != "blocklists" and dirname != "allowlists":
            start_response('404 NOT FOUND', [('Content-Type', 'text/plain')])
            return ['Not Found']

    if environ.get('REQUEST_METHOD', '') == "POST":
        post = cgi.FieldStorage(fp=environ['wsgi.input'], environ=environ, keep_blank_values=True)
        print >> sys.stderr, 'POST merge=%s\n' % post.getvalue('merge')

        # only import of addressbook supported
        if "import" not in params or params["import"] != "addressbook":
            start_response('404 NOT FOUND', [('Content-Type', 'text/plain')])
            return ['Not Found']

        merge_name = os.path.basename(post.getvalue('merge'))
        tmp_name = os.path.join("/tmp", post['uploadedfile'].filename)
        tmp_file = post['uploadedfile'].file
        print >> sys.stderr, 'POST tmp_name=%s\n' % tmp_name

        cmd = []
        extention = os.path.splitext(post.getvalue('name'))[1].lower()
        if extention == ".csv":
            cmd = [os.path.join(config.CALLBLOCKER_DATADIR, "import_CSV.py")]
        elif extention == ".ldif":
            cmd = [os.path.join(config.CALLBLOCKER_DATADIR, "import_LDIF.py")]
        elif extention == ".vcf":
            cmd = [os.path.join(config.CALLBLOCKER_DATADIR, "import_VCF.py")]
        print >> sys.stderr, 'cmd=%s\n' % cmd

        def get_contry_code():
            with open(SETTINGS_FILE) as f:
                jj = json.loads(f.read())
            phones = jj["phones"]
            return phones[0]["country_code"]

        exitCode = -1
        if len(cmd) != 0:
            cmd.append("--input"), cmd.append(tmp_name)
            cmd.append("--country_code"), cmd.append(get_contry_code())
            cmd.append("--merge"), cmd.append(os.path.join(config.CALLBLOCKER_SYSCONFDIR, dirname, merge_name))
            try:
                with open(tmp_name, 'wb') as f:
                    f.write(tmp_file.read())
                print >> sys.stderr, 'execute: %s\n' % cmd
                p = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
                out, err = p.communicate()
                exitCode = p.returncode
                if exitCode != 0:
                    print >> sys.stderr, 'out="%s"\n' % out
                    print >> sys.stderr, 'err="%s"\n' % err
            finally:
                os.remove(tmp_name)

        htmldata = {}
        if exitCode != 0:
            htmldata["ERROR"] = "Improper data sent - no files found"

        start_response('200 OK', [('Content-Type', 'text/json')])
        return [json.dumps(htmldata)]

    # get all files from directory
    files = []
    base = os.path.join(config.CALLBLOCKER_SYSCONFDIR, dirname)
    for fname in os.listdir(base):
        fullname = os.path.join(base, fname)
        if os.path.isfile(fullname) and os.path.splitext(fname)[1] == ".json":
            files.append(fullname)

    # to keep app.js simple
    main_fullname = os.path.join(base, "main.json")
    if not main_fullname in files:
        _create_empty_list(main_fullname, "main")
        files.append(main_fullname)

    all = []
    for fname in files:
        with open(fname, encoding="utf-8") as f:
            jj = json.loads(f.read())
        all.append({"name": jj["name"], "file": os.path.basename(fname)})

    all_count = len(all)

    # handle paging
    start = int(params.get("start", "0"))
    count = int(params.get("count", str(all_count)))

    items = []
    for i in range(start, all_count):
        if i >= start + count: break
        entry = all[i]
        items.append(entry)

    headers = [
        ('Content-Type',  'text/json'),
        ('Content-Range', 'items %d-%d/%d' % (start, start+count, all_count))
    ]
    start_response('200 OK', headers)
    return [json.dumps({"label": "name", "identifier": "file", "numRows": all_count, "items": items})]


def handle_get_online_scripts(environ, start_response, params):
    kind = "lookup"
    if "kind" in params:
        kind = params["kind"]

    script_prefix = "onlinecheck_"
    if kind == "lookup":
        script_prefix = "onlinelookup_"

    all = []
    for fname in os.listdir(config.CALLBLOCKER_DATADIR):
        if fname.startswith(script_prefix) and fname.endswith(".py"):
            script = fname[len(script_prefix):-3]
            all.append({"name": script, "value": script})
    all_count = len(all)

    # handle paging
    start = int(params.get("start", "0"))
    count = int(params.get("count", str(all_count)))

    items = []
    for i in range(start, all_count):
        if i >= start + count: break
        entry = all[i]
        items.append(entry)

    headers = [
        ('Content-Type',  'text/json'),
        ('Content-Range', 'items %d-%d/%d' % (start, start+count, all_count))
    ]
    start_response('200 OK', headers)
    return [json.dumps({"label": "name", "identifier": "value", "numRows": all_count, "items": items})]
