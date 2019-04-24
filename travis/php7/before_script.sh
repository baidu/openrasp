#!/bin/bash
set -ev
#mysql
mysql -e "CREATE USER 'openrasp'@'%' IDENTIFIED BY '123456'";
mysql -e "USE mysql;UPDATE user SET password=PASSWORD('rasp#2019') WHERE user='root';FLUSH PRIVILEGES;";
ln -s /run/mysqld/mysqld.sock /tmp/mysql.sock

#pgsql
psql -c "ALTER USER postgres WITH PASSWORD 'postgres';" -U postgres ;
psql -c "CREATE USER openrasp WITH PASSWORD '123456'" -U postgres;

pushd agent/$OPENRASP_LANG
phpenv config-rm xdebug.ini || true
phpenv config-rm ext-opcache.ini || true
wget -N https://packages.baidu.com/app/openrasp/libv8-5.9-linux.tar.gz -P ~/cache
tar -zxf ~/cache/libv8-5.9-linux.tar.gz
phpize && ./configure --with-v8=./libv8-5.9-linux --with-gettext --enable-openrasp-remote-manager --enable-cli-support && make -j2 --quiet
popd