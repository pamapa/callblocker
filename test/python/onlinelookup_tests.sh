#!/bin/bash

BASE_PATH=`dirname $0`
SCRIPT_PATH=$BASE_PATH/../../usr/share/callblocker

UNKNOWN_CID="{\"name\": \"\"}"
#echo $UNKNOWN_CID

echo "Executing onlinelookup tests..."

#python $SCRIPT_PATH/onlinelookup_dasschnelle_at.py --number +436642503442 --debug
#python $SCRIPT_PATH/onlinelookup_tel_search_ch.py --number +41265051361 --debug
#exit

echo "Execute onlinelookup_all.py tests..."
numbers='
  +33145537601
  +39885432087
  +41265051361
  +436642503442
  +49897557354
'
for number in $numbers; do
  res=`python $SCRIPT_PATH/onlinelookup_all.py --number $number`
  if [ $? -ne 0 ]; then
    echo "ERROR: script failed"
    exit -1
  fi
  echo $res
  if [ "$res" == "$UNKNOWN_CID" ]; then
    echo "ERROR: did not find"
  fi
done

exit 0

