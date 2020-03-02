#!/bin/bash

PROJECT_DIR=$(pwd)

BUILD_DIR=$PROJECT_DIR/build
XPL_DIR=x_gamepad/64
BIN_XPL_DIR=$BUILD_DIR/bin/XPL_DIR
BIN_XPL_FILES=$BUILD_DIR/bin/$XPL_DIR/*.xpl

DEPLOY_DIR=$PROJECT_DIR/deploy
DEPLOY_XPL_DIR=$DEPLOY_DIR/$XPL_DIR

mkdir -p $DEPLOY_XPL_DIR || exit $?

if [ $CROSS_TRIPLE = "x86_64-w64-mingw32" ]; then
  export CMAKE_SYSTEM_NAME="Windows"
  cp $PROJECT_DIR/lib/hidapi/hidapi.dll $DEPLOY_XPL_DIR || exit $?
elif [ $CROSS_TRIPLE = "x86_64-apple-darwin14" ]; then
  export CMAKE_SYSTEM_NAME="Darwin"
  STRIP_SYMBOLS=false
  ln -s /usr/osxcross/SDK/MacOSX10.10.sdk/System/Library /Library || exit $?
elif [ $CROSS_TRIPLE = "x86_64-linux-gnu" ]; then
  export CMAKE_SYSTEM_NAME="Linux"
else
  echo -e "\nError: unknown target" $CROSS_TRIPLE
  exit 1
fi

echo -e "\nStarting build for" $CMAKE_SYSTEM_NAME

mkdir -p $BUILD_DIR &&
cd $BUILD_DIR &&
rm -rf * &&
cmake -DCMAKE_SYSTEM_NAME=$CMAKE_SYSTEM_NAME .. &&
make || exit $?

cp -R $BIN_XPL_FILES $DEPLOY_XPL_DIR &&

echo -e "\nBuild successful!"