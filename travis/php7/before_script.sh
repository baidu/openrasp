#!/bin/bash
set -ev
#mysql
mysql -e "CREATE USER 'openrasp'@'%' IDENTIFIED BY '123456'";
mysql -e "USE mysql;UPDATE user SET authentication_string=PASSWORD('rasp#2019') WHERE user='root';FLUSH PRIVILEGES;";
ln -s /run/mysqld/mysqld.sock /tmp/mysql.sock

#pgsql
psql -c "ALTER USER postgres WITH PASSWORD 'postgres';" -U postgres ;
psql -c "CREATE USER openrasp WITH PASSWORD '123456'" -U postgres;

#cmake
wget -N https://cmake.org/files/v3.14/cmake-3.14.5-Linux-x86_64.tar.gz -P $HOME/cache
tar zxf $HOME/cache/cmake-3.14.5-Linux-x86_64.tar.gz -C /tmp
export PATH=/tmp/cmake-3.14.5-Linux-x86_64/bin:$PATH

#openrasp-v8
mkdir -p $TRAVIS_BUILD_DIR/openrasp-v8/build
pushd $TRAVIS_BUILD_DIR/openrasp-v8/build
cmake -DENABLE_LANGUAGES=php ..
make -j2 --quiet
popd

pushd agent/$OPENRASP_LANG
phpenv config-rm xdebug.ini || true
phpenv config-rm ext-opcache.ini || true
phpize && ./configure --with-openrasp-v8=$TRAVIS_BUILD_DIR/openrasp-v8 --with-gettext --enable-openrasp-remote-manager --enable-cli-support && make -j2
popd