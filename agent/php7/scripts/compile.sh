#!/bin/bash

cd $(dirname "$0")/../
set -ex

if [[ ! -d "/tmp/libv8" ]]; then
    wget https://packages.baidu.com/app/openrasp/libv8-5.9-linux.tar.gz -P /tmp
    tar -xvf /tmp/libv8-5.9-linux.tar.gz -C /tmp
fi

phpize --clean
phpize
./configure --with-v8=/tmp/libv8 -q
make
