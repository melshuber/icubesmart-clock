#! /bin/sh

set -ex

stty 115200 -F ${1:-/dev/ttyUSB0}
echo 'R' > ${1:-/dev/ttyUSB0}
