#!/bin/bash

check_command() {
  if hash $1 2>/dev/null; then
    echo "'$1' found."
  else
    echo "ERROR: '$1' is missing. Please fix."
    exit 1
  fi
}

if_cmd_failed() {
  if [ $? -ne 0 ]; then
    if [ -n $1 ]; then
      echo "ERROR: $1";
    fi
    exit 1;
  fi
}

FLAG_REBUILD=

for arg in $@; do
  if [ "$arg" == "--rebuild" ]; then
    FLAG_REBUILD=1
  fi
done

# Some quick checks just in case...
check_command autoreconf
check_command aclocal
check_command libtoolize
check_command git

if [ -z $INFORMIXDIR ]; then
  echo "\$INFORMIXDIR is not set"
  exit 1
fi

UDR_INSTALL_PATH=$INFORMIXDIR/extend/informix_spark_streaming.so
if [ -a $UDR_INSTALL_PATH ]; then
  echo "'$UDR_INSTALL_PATH' already exists!"
  exit 1
fi

MAIN_DIR=`pwd`
LIB_DIR=$MAIN_DIR/lib

echo "Current dir: $MAIN_DIR"
echo "Lib dir: $LIB_DIR"

if [ -d ./lib ]; then
  echo "./lib already exists... Removing..."
  rm -rf ./lib
  if_cmd_failed "Unable to remove ./lib"
fi

mkdir lib

echo -e "--------------------------------------";
echo -e "Retrieving and building wolfSSL...";
echo -e "--------------------------------------\n";

if [ -d ./wolfssl ]; then
  cd wolfssl
  git status > /dev/null
  if [ $? -ne 0 ]; then
    echo "./wolfssl already exist, but is not a Git repository... Removing...";
    cd ..
    rm -rf ./wolfssl
    if_cmd_failed "Unable to remove ./wolfssl"
  else
    cd ..
  fi
fi

if [ ! -d ./wolfssl ]; then
  git clone http://github.com/wolfSSL/wolfssl
  if_cmd_failed "Unable to clone wolfssl Git repository"
fi

cd wolfssl
OUTPUT=`git branch --list "temp_build_branch"`
if [ "$OUTPUT" != "" ]; then
  if [ $FLAG_REBUILD ]; then
    git checkout -q master
    git branch -D temp_build_branch
    if_cmd_failed "Unable to delete the build branch."
    git checkout -b temp_build_branch v3.8.0
    if_cmd_failed "Unable to create temp build branch."
  else
    git checkout -q temp_build_branch
  fi
fi

if [ $FLAG_REBUILD ] || [ ! -f ./configure -o ! -f ./Makefile.in ]; then
  ./autogen.sh
  if_cmd_failed
fi
if [ $FLAG_REBUILD ] || [ ! -f ./Makefile ]; then
  echo "Running configure"
  ./configure --prefix=$LIB_DIR
  if_cmd_failed
fi
make
if_cmd_failed
make install
if_cmd_failed

cd ..

echo -e "--------------------------------------";
echo -e "Retrieving and building wolfMQTT...";
echo -e "--------------------------------------\n";

if [ -d ./wolfMQTT ]; then
  cd wolfMQTT
  git status > /dev/null
  if [ $? -ne 0 ]; then
    echo "./wolfMQTT already exist, but is not a Git repository... Removing...";
    cd ..
    rm -rf ./wolfMQTT
    if_cmd_failed "Unable to remove ./wolfMQTT"
  else
    cd ..
  fi
fi

if [ ! -d ./wolfMQTT ]; then
  git clone http://github.com/wolfSSL/wolfMQTT
  if_cmd_failed "Unable to clone wolfMQTT Git repository"
fi

cd wolfMQTT
OUTPUT=`git branch --list "temp_build_branch"`
if [ "$OUTPUT" != "" ]; then
  if [ $FLAG_REBUILD ]; then
    git checkout -q master
    git branch -D temp_build_branch
    if_cmd_failed "Unable to delete the build branch."
    git checkout -b temp_build_branch v0.4
    if_cmd_failed "Unable to create temp build branch."
  else
    git checkout -q temp_build_branch
  fi
fi

if [ $FLAG_REBUILD ] || [ ! -f ./configure -o ! -f ./Makefile.in ]; then
  ./autogen.sh
  if_cmd_failed
fi
if [ $FLAG_REBUILD ] || [ ! -f ./Makefile ]; then
  ./configure --prefix=$LIB_DIR
  if_cmd_failed
fi
make
if_cmd_failed
make install
if_cmd_failed

cd ..

echo -e "--------------------------------------";
echo -e "Building UDR..."
echo -e "--------------------------------------\n";

gcc -Wall -fPIC -DMI_SERVBUILD -I$LIB_DIR/include -I$INFORMIXDIR/incl/public -I$INFORMIXDIR/incl -c *.c
if_cmd_failed
gcc -Wl,-rpath=$LIB_DIR/lib,-L$LIB_DIR/lib -lwolfssl -lwolfmqtt *.o -shared -o informix_spark_streaming.so
if_cmd_failed
cp ./informix_spark_streaming.so $UDR_INSTALL_PATH
if_cmd_failed "Unable to copy library to $UDR_INSTALL_PATH"

echo -e "\n--------------------------------------";
echo -e "DONE.";
echo -e "--------------------------------------\n";