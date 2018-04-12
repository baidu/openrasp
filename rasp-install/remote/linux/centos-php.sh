#!/bin/bash

set -ex

wget https://packages.baidu.com/app/openrasp/rasp-php.tar.bz2 -O - | tar -xjvf -
cd rasp-php-20*
php install.php -d /opt/rasp
apachectl -k restart

