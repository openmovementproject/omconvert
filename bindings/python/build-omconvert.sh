#!/bin/bash
set -e
mkdir omconvert.build
curl -L https://github.com/digitalinteraction/omconvert/archive/master.zip -o omconvert.build/master.zip
unzip omconvert.build/master.zip -d omconvert.build
make -C omconvert.build/omconvert-master/src/omconvert
cp omconvert.build/omconvert-master/src/omconvert/omconvert .
