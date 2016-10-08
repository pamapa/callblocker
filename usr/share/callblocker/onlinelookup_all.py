#!/usr/bin/env python

# callblocker - blocking unwanted calls from your home phone
# Copyright (C) 2015-2016 Patrick Ammann <pammann@gmx.net>
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
import sys

from online_base import OnlineBase
from onlinelookup_tel_search_ch import OnlineLookupTelSearchCH
from onlinelookup_dasoertliche_de import OnlineLookupDasOertlicheDE
from onlinelookup_dasschnelle_at import OnlineLookupDasSchnelleAT


#
# main
#
if __name__ == "__main__":
    base = OnlineBase()
    parser = base.get_parser("Online lookup via all supported sources")
    args = parser.parse_args()

    all = [
        OnlineLookupTelSearchCH(),
        OnlineLookupDasOertlicheDE(),
        OnlineLookupDasSchnelleAT()
    ]

    for m in all:
        for cc in m.supported_country_codes():
            if args.number.startswith(cc):
                m.run(args)

    # m.run would have exited
    base.log.error("number '%s' is not supported" % args.number)
    sys.exit(-1)
