#!/bin/bash
set -ev

pushd agent/php
php run-tests.php -p `which php` -d extension=`pwd`/modules/openrasp.so --offline --show-diff --set-timeout 120
popd