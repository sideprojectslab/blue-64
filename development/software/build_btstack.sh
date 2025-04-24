#!/bin/bash

cwd=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
dst=$cwd/src
export IDF_PATH=$dst

rm -rf $dst/components/btstack

cd $cwd/../../resources/bluepad32/external/btstack
git apply ../patches/*.patch

cd $cwd/../../resources/bluepad32/external/btstack/port/esp32
python integrate_btstack.py

cd $cwd/../../resources/bluepad32/external/btstack/src
git restore .

cd $cwd

read -n 1 -s -r -p "Press any key to continue..."
