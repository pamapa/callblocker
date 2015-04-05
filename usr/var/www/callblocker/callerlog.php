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
  include("settings.php");

  $data = shell_exec(CALLBLOCKER_CALLLOGCMD);
  $all = array_filter(explode("\n", trim($data)));
  $all_count = count($all);

  $start = 0;
  $count = $all_count;

  // Handle paging, if given.
  if (array_key_exists("start", $_REQUEST)) {
    $start = $_REQUEST["start"];
  }
  if (array_key_exists("count", $_REQUEST)) {
    $count = $_REQUEST["count"];
  }

  $ret = array();
  for ($i = $start; $i < $start+$count && $i < $all_count; $i++) {
    $entry = $all[$all_count - $i - 1]; // newest first
    $json = json_decode($entry);

    $re_sq = "'([^'\\\\]*(?:\\\\.[^'\\\\]*)*)'"; // support escaped quotes
    if (preg_match("/^Incoming call: (number=".$re_sq.")?\s?(name=".$re_sq.")?\s?(blocked)?\s?".
                   "(whitelist=".$re_sq.")?\s?(blacklist=".$re_sq.")?\s?(score=([0-9]*))?$/",
                   $json->{"MESSAGE"}, $matches)) {
      //var_dump($matches);
      $matches_count = count($matches);

      $number    = stripslashes($matches[2]);
      $name      = "";
      $blocked   = "";
      $blacklist = "";
      $whitelist = "";
      $score     = "";

      if ($matches_count > 4) {
        $name = stripslashes($matches[4]);
      }
      if ($matches_count > 5) {
        $blocked = stripslashes($matches[5]);
      }
      if ($matches_count > 7) {
        $whitelist = stripslashes($matches[7]);
      }
      if ($matches_count > 9) {
        $blacklist = stripslashes($matches[9]);
      }
      if ($matches_count > 11) {
        $score = $matches[11];
      }

      $tmp = array(
        "TIMESTAMP"=>$json->{"__REALTIME_TIMESTAMP"}/1000, // usec -> ms
        "NUMBER"=>$number, "NAME"=>$name, "BLOCKED"=>$blocked,
        "WHITELIST"=>$whitelist, "BLACKLIST"=>$blacklist, "SCORE"=>$score
      );
      array_push($ret, $tmp);
    }
  }

  header("Content-Type", "text/json");
  header(sprintf("Content-Range: items %d-%d/%d", $start, $start+$count, $all_count));
  print json_encode(array("numRows"=>$all_count, "items"=>$ret), JSON_PRETTY_PRINT);
?>

