#!/bin/bash

if [[ -d output ]]; then
  rm output/*
else
  mkdir output
fi
cd output
split -d -b 480000 ../output.bin img
for entry in *
do
  convert -size 800x600 -depth 8 -define quantum:format=integer gray:$entry -normalize $entry.png
  echo "converted $entry -> $entry.png"
done