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

  function scanListFiles($dirName) {
    $files = scandir(CALLBLOCKER_SYSCONFDIR."/".$dirName);
    $ret = array();
    foreach($files as $f) {
      $file = CALLBLOCKER_SYSCONFDIR."/".$dirName."/".$f;
      if (pathinfo($file, PATHINFO_EXTENSION) == "json") {
        array_push($ret, $file);
      }
    }
    return $ret;
  }

  function getContryCode() {
    $file = CALLBLOCKER_SYSCONFDIR."/settings.json";
    $json = json_decode(file_get_contents($file));
    //var_dump($json);
    $phones = $json->{"phones"};
    return $phones[0]->{"country_code"};
  }

  $dirname = "blacklists";
  if (array_key_exists("dirname", $_REQUEST)) {
    $dirname = $_REQUEST["dirname"];
    if ($dirname != "blacklists" and $dirname != "whitelists") {
      http_response_code(404);
      return;
    }
  }

/*
  foreach ($_POST as $key => $value) {
    error_log("POST ".$key.": ".$value);
  }

  foreach ($_REQUEST as $key => $value) {
    error_log("REQUEST ".$key.": ".$value);
  }

  foreach ($_FILES as $key1 => $entry) {
    if (is_array($entry)) {
        foreach($entry as $key2 => $value) {
           error_log("FILES ".$key1 . ": " .$key2 . ": " . $value);
        }
     } else {
        error_log("FILES ".$key1 . ": " . $entry);
     }
  }
*/

  if (array_key_exists("name", $_POST)) {
    $merge_name = basename($_REQUEST["merge"]);
    $tmp_name = $_FILES["uploadedfile"]["tmp_name"];
    
    $cmd = "";
    if (pathinfo($_POST["name"], PATHINFO_EXTENSION) == "csv") {
      $cmd = "python ".CALLBLOCKER_DATADIR."/convert_CSV.py";
    } else if (pathinfo($_POST["name"], PATHINFO_EXTENSION) == "ldif") {
      $cmd = "python ".CALLBLOCKER_DATADIR."/convert_LDIF.py";
    }

    $exitCode = -1;
    $output = [];
    if ($cmd != "") {
      $cmd .= " --input ".$tmp_name;
      $cmd .= " --country_code " . getContryCode();
      $cmd .= " --merge ".CALLBLOCKER_SYSCONFDIR."/".$dirname."/".$merge_name;
      $cmd .= " 2>&1";
      //error_log("execute: ". $cmd);
      exec($cmd, $output, $exitCode);
    }
    
    $htmldata = array();
    if ($exitCode != 0) {
      error_log($cmd." failed with ".$exitCode);
      foreach($output as $o) {
        error_log($o);
      }
      $htmldata = array("ERROR" => "Improper data sent - no files found");
    } else {
      $htmldata[0] = $_POST;
    }
    header("Content-Type", "text/json");
    print json_encode($htmldata, JSON_PRETTY_PRINT);
    return;
  }

  $files = scanListFiles($dirname);
  $main_found = False;
  $all = array();
  foreach($files as $f) {
    $json = json_decode(file_get_contents($f));
    if (strcmp(basename($f), "main.json") == 0) $main_found = True;
    $tmp = array("name"=>$json->{"name"}, "file"=>basename($f));
    array_push($all, $tmp);
  }
  // we need at least "main.json" entry to keep app.js simple
  if (!$main_found) {
    $tmp = array("name"=>"main", "file"=>"main.json");
    array_push($all, $tmp);
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
  print json_encode(array("identifier"=>"file", "label"=>"name", "numRows"=>$all_count, "items"=>$ret), JSON_PRETTY_PRINT);
?>

