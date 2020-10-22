#!/bin/bash

if [ -z "$1" ] ; then
  echo "usage: currmon-demo.sh <BoardID>"
  exit 1
fi

echo ./currmon-admin GetStatus
./currmon-admin GetStatus

mkdir ${1}_logs
echo Setting log path
./currmon-admin SetLogPath -b ${1} -l ${PWD}/${1}_logs

echo Getting log path
./currmon-admin GetLogPath -b ${1}

echo Setting min trigger duration
./currmon-admin SetMinDuration -b ${1} -d 5

echo Getting min trigger duration
./currmon-admin GetMinDuration -b ${1}

echo Setting max trigger duration
./currmon-admin SetMaxDuration -b ${1} -d 30

echo Getting min trigger duration
./currmon-admin GetMaxDuration -b ${1}

echo Setting manual trigger
./currmon-admin SetTrigger -b ${1} -c 3

echo Sleeping for 10 seconds
sleep 10

echo clearning manual trigger
./currmon-admin ClearTrigger -b ${1} -c 3
