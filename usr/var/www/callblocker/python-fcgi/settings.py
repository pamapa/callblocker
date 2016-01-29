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
import cgi
from datetime import datetime

import config


SETTINGS_FILE = os.path.join(config.CALLBLOCKER_SYSCONFDIR, "settings.json")


def handle_phones(environ, start_response, params):
  with open(SETTINGS_FILE) as f:
    jj = json.load(f)

  if environ.get('REQUEST_METHOD', '') == "POST":
    post = cgi.FieldStorage(fp=environ['wsgi.input'], environ=environ, keep_blank_values=True)
    #print >> sys.stderr, 'POST data=%s\n' % post.getvalue('data')    
    json_phones = json.loads(post.getvalue('data'))
    # Analog vs. SIP: remove "device" or others
    for phone in json_phones["items"]:
      if phone["device"] == "":
        # mark as SIP
        del phone["device"]
      else:
        # mark as Analog
        del phone["from_domain"]
        del phone["from_username"]
        del phone["from_password"]
    jj["phones"] = json_phones["items"]
    with open(SETTINGS_FILE, 'w') as f:
      json.dump(jj, f, indent=4)
    return

  all = []
  if "phones" in jj:
    all = jj["phones"]
  # we need at least one default entry to keep app.js simple
  if len(all) == 0:
    entry = {"enabled": False, "name": "My Home Phone"}
    all.append(entry)
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
  with open(SETTINGS_FILE) as f:
    jj = json.load(f)

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
  dirname = "blacklists"
  if "dirname" in params:
    dirname = params["dirname"]
    if dirname != "blacklists" and dirname != "whitelists":
      start_response('404 NOT FOUND', [('Content-Type', 'text/plain')])
      return ['Not Found']

  filename = "main.json"
  if "filename" in params:
    filename = os.path.basename(params["filename"])
  filename = os.path.join(config.CALLBLOCKER_SYSCONFDIR, dirname, filename)
  
  if environ.get('REQUEST_METHOD', '') == "POST":
    post = cgi.FieldStorage(fp=environ['wsgi.input'], environ=environ, keep_blank_values=True)
    #print >> sys.stderr, 'POST data=%s\n' % post.getvalue('data')
    json_list = json.loads(post.getvalue('data'))
    with open(filename, 'w') as f:
      json.dump({"name": json_list["label"], "entries": json_list["items"]}, f, indent=4)
    return

  with open(filename) as f:
    jj = json.load(f)
  all = jj["entries"]
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
    label = os.path.splitext(filename)[0]

  headers = [
    ('Content-Type',  'text/json'),
    ('Content-Range', 'items %d-%d/%d' % (start, start+count, all_count))
  ]
  start_response('200 OK', headers)
  return [json.dumps({"label": label, "numRows": all_count, "items": items})]


def handle_get_lists(environ, start_response, params):
  dirname = "blacklists"
  if "dirname" in params:
    dirname = params["dirname"]
    if dirname != "blacklists" and dirname != "whitelists":
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
    extention = os.path.splitext(post.getvalue('name'))[1]
    if extention == ".csv":
      cmd = ["python", os.path.join(config.CALLBLOCKER_DATADIR, "import_CSV.py")]
    elif extention == ".ldif":
      cmd = ["python", os.path.join(config.CALLBLOCKER_DATADIR, "import_LDIF.py")]
    elif extention == ".vcf":
      cmd = ["python", os.path.join(config.CALLBLOCKER_DATADIR, "import_VCARD.py")]
    print >> sys.stderr, 'cmd=%s\n' % cmd

    def get_contry_code():
      with open(SETTINGS_FILE) as f:
        jj = json.load(f)
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
  for f in os.listdir(base):
    file = os.path.join(base, f)
    if os.path.isfile(file) and os.path.splitext(f)[1] == ".json":
      files.append(file)

  main_found = False

  all = []
  for file in files:
    with open(file) as f:
      jj = json.load(f)
    if os.path.basename(file) == "main.json": main_found = True
    all.append({"name": jj["name"], "file": os.path.basename(file)})

  # we need at least "main.json" entry to keep app.js simple
  if not main_found:
    all.append({"name": "main", "file": "main.json"})

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
  return [json.dumps({"identifier": "file", "label": "name", "numRows": all_count, "items": items})]

