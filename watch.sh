#!/usr/bin/env bash

./msg

inotifywait -q -m -r -e modify $(cat config.h | grep DIRECTORY | cut -d '"' -f 2) | while read; do
  rm -rf dist
  ./msg
  echo $?
done
