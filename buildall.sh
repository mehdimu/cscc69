#!/bin/sh
set -e

cd A2/a2/kern/conf
./config ASST2-NORAND
cd ../compile/ASST2-NORAND
bmake depend
bmake
bmake install
cd ../../..
bmake
bmake install

