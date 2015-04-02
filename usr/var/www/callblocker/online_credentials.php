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

  # dojo json exchange format see:
  # http://dojotoolkit.org/reference-guide/1.10/dojo/data/ItemFileReadStore.html#input-data-format

  $file = CALLBLOCKER_SYSCONFDIR."/settings.json";
  //print $file;

  if (array_key_exists("data", $_POST)) {
    //error_log("POST data:".$_POST["data"]);
    $json_creds = json_decode($_POST["data"]);
    $json = json_decode(file_get_contents($file));

    $json->{"online_credentials"} = $json_creds->{"items"};
    error_log("online_credentials:\n".json_encode($json, JSON_PRETTY_PRINT));
    file_put_contents($file, json_encode($json, JSON_PRETTY_PRINT));
    return;
  }

  $json = json_decode(file_get_contents($file));
  //var_dump($json);
  $all = array();
  if ($json->{"online_credentials"} != null) {
    $all = $json->{"online_credentials"};
  }
  //var_dump($all);
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
    $entry = $all[$i];
    array_push($ret, $entry);
  }

  header("Content-Type", "text/json");
  header(sprintf("Content-Range: items %d-%d/%d", $start, $start+$count, $all_count));
  print json_encode(array("numRows"=>$all_count, "items"=>$ret), JSON_PRETTY_PRINT);
?>

