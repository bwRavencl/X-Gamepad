#!/bin/bash

declare -a TARGETS=("x86_64-w64-mingw32" "x86_64-apple-darwin" "x86_64-linux-gnu")

for TARGET in ${TARGETS[@]}; do
  docker run -it --rm -v $(pwd):/workdir -e CROSS_TRIPLE=$TARGET multiarch/crossbuild ./build.sh
done