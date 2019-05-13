#!/bin/bash
set -ev
#mysql
mysql -e "CREATE USER 'openrasp'@'%' IDENTIFIED BY '123456'";
mysql -e "USE mysql;UPDATE user SET password=PASSWORD('rasp#2019') WHERE user='root';FLUSH PRIVILEGES;";
ln -s /run/mysqld/mysqld.sock /tmp/mysql.sock

#pgsql
psql -c "ALTER USER postgres WITH PASSWORD 'postgres';" -U postgres ;
psql -c "CREATE USER openrasp WITH PASSWORD '123456'" -U postgres;

#openrasp-v8
git clone --depth=1 https://github.com/baidu-security/openrasp-v8.git /tmp/openrasp-v8
mkdir -p /tmp/openrasp-v8/php/build
pushd /tmp/openrasp-v8/php/build
cmake ..
make -j2 --quiet
popd

#openrasp
pushd agent/$OPENRASP_LANG
phpenv config-rm xdebug.ini || true
phpenv config-rm ext-opcache.ini || true
phpize && ./configure --with-openrasp-v8=/tmp/openrasp-v8 --with-gettext --enable-openrasp-remote-manager --enable-cli-support && make -j2
popd