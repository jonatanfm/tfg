#!/bin/bash

TARGET=shaders.h

echo "" > $TARGET
echo "// This file is generated automatically." >> $TARGET 
echo "" >> $TARGET

for file in *.{frag,vert}; do
  name="${file%%.*}"
  ext="${file##*.}"

  type=${ext,,}
  type=${type/vert/VERTEX}
  type=${type/frag/FRAGMENT}
    
  echo "const char SHADER_${name^^}_${type}[] = \"\"" >> $TARGET
  sed 's/^.*$/\"\0\\n\"/g' < $file >> $TARGET
  echo ";" >> $TARGET
  echo "" >> $TARGET
done
