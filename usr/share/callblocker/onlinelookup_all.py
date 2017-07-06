#!/usr/bin/env python3

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

import os, sys
import logging

from online_base import OnlineBase


#
# main
#
if __name__ == "__main__":
    base = OnlineBase()
    parser = base.get_parser("Online lookup via all supported sources")
    args = parser.parse_args()

    if args.debug: base.log.setLevel(logging.DEBUG)
    
    all = []

    # dynamic use all "onlinelookup_x.py" modules
    path = os.path.dirname(os.path.realpath(__file__))
    base.log.debug("search in: %s" % path)
    for fn in os.listdir(path):
        if not os.path.isfile(os.path.join(path, fn)): continue
        if not fn.startswith("onlinelookup_"): continue
        if not fn.endswith(".py"): continue
        if fn == "onlinelookup_all.py": continue
        module_name = os.path.splitext(fn)[0]
        base.log.debug("try to load: %s" % module_name)
        found = False
        try:
            module = __import__(module_name)
            for key in dir(module):
                if key == "OnlineBase": continue
                try:
                    olClass = getattr(module, key)
                    if issubclass(olClass, OnlineBase):
                        base.log.debug("use module %s" % module_name)
                        all.append(olClass())
                        found = True
                except TypeError: pass
        except: pass
        if not found:
            base.log.warn("failed to load/use %s" % module_name)
    
    # lookup
    for m in all:
        for cc in m.supported_country_codes():
            if args.number.startswith(cc):
                m.run(args)

    # m.run would have exited
    base.log.debug("number '%s' is not supported" % args.number)
    sys.exit(-1)
