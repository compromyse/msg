#!/usr/bin/env bash

inotifywait -q -m -r -e modify $1 | while read DIRECTORY EVENT FILE; do
  rm -rf dist && ./msg
done
