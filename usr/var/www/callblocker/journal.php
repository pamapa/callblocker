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
  include("config.php");

  function mapPriorityToName($prio) {
    switch ($prio) {
      case  0: return "Emergency";
      case  1: return "Alert";
      case  2: return "Critical";
      case  3: return "Error";
      case  4: return "Warning";
      case  5: return "Notice";
      case  6: return "Information";
      case  7: return "Debug";
      default: return "Unknown";
    }
  }

  $data = "";
  if (array_key_exists("all", $_REQUEST)) {
    $data = shell_exec(CALLBLOCKER_JOURNALALL);
  } else {
    $data = shell_exec(CALLBLOCKER_JOURNALERRORWARN);
  }
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
    //var_dump($json);

    date_default_timezone_set("UTC");
    $tmp = array(
      // -6: usec -> sec => UTC time (substr because timestamp is too big for int)
      "TIMESTAMP"=>date("Y-m-d H:i:s +0000", substr($json->{"__REALTIME_TIMESTAMP"}, 0, -6)),
      "PRIO_ID"=>intval($json->{"PRIORITY"}),
      "PRIORITY"=>mapPriorityToName($json->{"PRIORITY"}),
      "MESSAGE"=>$json->{"MESSAGE"}
    );
    array_push($ret, $tmp);
  }

  header("Content-Type", "text/json");
  header(sprintf("Content-Range: items %d-%d/%d", $start, $start+$count, $all_count));
  print json_encode(array("numRows"=>$all_count, "items"=>$ret), JSON_PRETTY_PRINT);
?>

