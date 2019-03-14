#!/bin/bash
set -ev

pushd agent/$OPENRASP_LANG
phpenv config-rm xdebug.ini || true
phpenv config-rm ext-opcache.ini || true
wget -N https://packages.baidu.com/app/openrasp/libv8-5.9-linux.tar.gz -P ~/cache
tar -zxf ~/cache/libv8-5.9-linux.tar.gz
phpize && ./configure --with-v8=./libv8-5.9-linux --with-gettext --enable-openrasp-remote-manager --enable-cli-support && make -j2 --quiet
popd