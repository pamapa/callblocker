#!/usr/bin/env python3

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2019-2019 Patrick Ammann <pammann@gmx.net>
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
import argparse
import socket
import flask
import urllib

import settings
import logs


app = flask.Flask("callblocker")
api_base = "/python-fcgi/api.py"

def wsgi_handler(func):
    environ = flask.request.environ
    params = dict(urllib.parse.parse_qsl(environ.get("QUERY_STRING", "")))
    headers = None
    def start_response(a, h):
        nonlocal headers
        headers = h
    data = func(environ, start_response, params)
    resp = flask.make_response(data[0])
    resp.headers = headers
    return resp

@app.route(api_base + "/phones")
def phones():
    return wsgi_handler(settings.handle_phones)
@app.route(api_base + "/online_credentials")
def online_credentials():
    return wsgi_handler(settings.handle_online_credentials)
@app.route(api_base + "/get_list")
def get_list():
    return wsgi_handler(settings.handle_get_list)
@app.route(api_base + "/get_lists")
def get_lists():
    return wsgi_handler(settings.handle_phones)
    (environ, params) = get_environ_and_params()
    data = settings.handle_get_lists(environ, start_response, params)
    resp = flask.make_response(data[0])
    resp.headers = g_headers
    return resp
@app.route(api_base + "/get_online_scripts")
def get_online_scripts():
    return wsgi_handler(settings.handle_get_online_scripts)

@app.route(api_base + "/callerlog")
def callerlog():
    return wsgi_handler(logs.handle_callerlog)
@app.route(api_base + "/journal")
def journal():
    return wsgi_handler(logs.handle_journal)

@app.route("/", defaults={"filename": "index.html"})
@app.route("/<path:filename>")
def index(filename):
  path = os.path.join("..", filename)
  return flask.send_file(path)


if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="callblocker web interface via flask")
  parser.add_argument("--debug", action="store_true")
  args = parser.parse_args()

  app.run(host="0.0.0.0", port=5000, debug=args.debug)

