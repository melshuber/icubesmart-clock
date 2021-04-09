#! /bin/sh

set -ex

stty 115200 -F ${1:-/dev/ttyUSB0}
echo "S:$(date +%Y%m%d%H%M%S)" > ${1:-/dev/ttyUSB0}
