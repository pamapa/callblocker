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
  header("Content-Type", "text/json");

  //$lines = shell_exec("journalctl _SYSTEMD_UNIT=callblockerd.service --since=-1w --priority=5");
  $data = shell_exec("journalctl --lines=1000 --output json");
  $all = explode("\n", trim($data));
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

  header(sprintf("Content-Range: items %d-%d/%d", $start, $start+$count, $all_count));

  // test
  // http://www.phpliveregex.com/
  $all = array();
  array_push($all, "Incoming call number='+1'");
  array_push($all, "Incoming call number='+2' name='y'");
  array_push($all, "Incoming call number='+3' name='ydssdsdsd\'33333' blocked");
  array_push($all, "Incoming call number='+41333333333' name='123456789012345678901234567890' blocked blacklist='12345678901234567890'");
  array_push($all, "Incoming call number='+5' name='y bs=\\ww' blocked blacklist='b' score=10");
  array_push($all, "Incoming call number='+6' name='y \"' whitelist='12345678901234567890'");
  $all_count = count($all);
  //var_dump($all);

  $ret = array();
  for ($i = 0; $i < count($all); $i++) {
    $msg = $all[$i];
    //echo $msg . "\n";
    $re_sq = "'([^'\\\\]*(?:\\\\.[^'\\\\]*)*)'"; // support escaped quotes
    if (preg_match("/^Incoming call (number=".$re_sq.")?\s?(name=".$re_sq.")?\s?(blocked)?\s?".
                   "(whitelist=".$re_sq.")?\s?(blacklist=".$re_sq.")?\s?(score=([0-9]*))?$/",
                   $msg, $matches)) {
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
        "DATE"=>gmdate("Y-m-d H:i:s", 1000000), // usec -> s
        "NUMBER"=>$number, "NAME"=>$name, "BLOCKED"=>$blocked,
        "WHITELIST"=>$whitelist, "BLACKLIST"=>$blacklist, "SCORE"=>$score
      );
      array_push($ret, $tmp);
    }
  }


/*
  $ret = array();
  for ($i = $start; $i < $start+$count && $i < $all_count; $i++) {
    $entry = $all[$all_count - $i - 1]; // newest first
    $obj = json_decode($entry);

    // Incoming call number='x' name='y' [blocked] [whitelist='w'] [blacklist='b'] [score=s]
    // split $obj->{"MESSAGE"} -> ....


    $tmp = array(
      "DATE"=>gmdate("Y-m-d H:i:s", $obj->{"__REALTIME_TIMESTAMP"}/1000000), // usec -> s
      "MESSAGE"=>$obj->{"MESSAGE"}
    );
    array_push($ret, $tmp);
  }
*/

  print json_encode(array("numRows"=>$all_count, "items"=>$ret), JSON_PRETTY_PRINT);
?>

