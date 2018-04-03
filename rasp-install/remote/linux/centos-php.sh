#!/bin/bash

wget https://packages.baidu.com/app/openrasp/rasp-php.tar.bz2
tar -xvf rasp-php.tar.bz2
cd rasp-php-20*
php install.php -d /opt/rasp
apachectl -k restart

