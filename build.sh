#!/bin/sh

clear
set -ex

./autogen.sh > dev/logs/autogen.sh.log 2>&1
./configure > dev/logs/configure.log 2>&1
make > dev/logs/make.log 2>&1
sudo make install > dev/logs/make-install.log 2>&1