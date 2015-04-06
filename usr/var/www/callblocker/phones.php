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

  # dojo json exchange format see:
  # http://dojotoolkit.org/reference-guide/1.10/dojo/data/ItemFileReadStore.html#input-data-format

  $file = CALLBLOCKER_SYSCONFDIR."/settings.json";
  //print $file;

  if (array_key_exists("data", $_POST)) {
    //error_log("POST data:".$_POST["data"]);
    $json_phones = json_decode($_POST["data"]);
    $json = json_decode(file_get_contents($file));

    // Analog vs. SIP: remove "device" or others
    foreach($json_phones->{"items"} as $phone) {
      if ($phone->{"device"} == "") {
        unset($phone->{"device"});
      } else {
        unset($phone->{"from_domain"});
        unset($phone->{"from_username"});
        unset($phone->{"from_password"});
      }
    }
    $json->{"phones"} = $json_phones->{"items"};
    //error_log("json:\n".json_encode($json, JSON_PRETTY_PRINT));
    file_put_contents($file, json_encode($json, JSON_PRETTY_PRINT));
    return;
  }

  $json = json_decode(file_get_contents($file));
  //var_dump($json);
  $all = array();
  if ($json->{"phones"} != null) {
    $all = $json->{"phones"};
  }
  //var_dump($all);
  $all_count = count($all);

  // TODO assign first enabled entry...
  $ret = array();
  for ($i = 0; $i < 1 && $i < $all_count; $i++) {
    $entry = $all[$i];
    array_push($ret, $entry);
  }

  // we need at least one default entry
  if (count($ret) == 0) {
    $entry = array(
      "enabled"=>true, "name"=>"My Home Phone"
    );
    array_push($ret, $entry);
  }

  header("Content-Type", "text/json");
  header(sprintf("Content-Range: items %d-%d/%d", 0, 0, 1));
  print json_encode(array("numRows"=>1, "items"=>$ret), JSON_PRETTY_PRINT);
?>

