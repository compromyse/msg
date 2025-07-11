#!/usr/bin/env bash

./build/msg
echo $?

inotifywait -q -m -r -e modify $(cat config.h | grep DIRECTORY | cut -d '"' -f 2) | while read; do
  rm -rf dist
  ./build/msg
  echo $?
done
