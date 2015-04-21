<?php
/*
 callblocker - blocking unwanted calls from your home phone
 Copyright (C) 2015-2015 Patrick Ammann <pammann@gmx.net>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 3
 of the License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

  define("CALLBLOCKER_SYSCONFDIR",       "/etc/callblocker");
  define("CALLBLOCKER_DATADIR",          "/usr/share/callblocker");

  // logging via journald
  define("CALLBLOCKER_CALLLOGCMD",       "journalctl _SYSTEMD_UNIT=callblockerd.service --priority=5..5 --lines=1000 --output json");
  define("CALLBLOCKER_JOURNALALL",       "journalctl _SYSTEMD_UNIT=callblockerd.service --lines=1000 --output json");
  define("CALLBLOCKER_JOURNALERRORWARN", "journalctl _SYSTEMD_UNIT=callblockerd.service --priority=0..4 --lines=1000 --output json");
?>

