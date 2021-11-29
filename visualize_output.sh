#!/bin/bash

if [[ -d output ]]; then
  rm output/*
else
  mkdir output
fi
cd output
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
  #Linux
  split -d -b 480000 ../output.bin img
elif [[ "$OSTYPE" == "darwin"* ]]; then
  #MacOS does not support numeric-suffix
  split -b 480000 ../output.bin img
fi
for entry in *
do
  convert -size 800x600 -depth 8 -define quantum:format=integer gray:$entry -normalize $entry.png
  echo "converted $entry -> $entry.png"
done