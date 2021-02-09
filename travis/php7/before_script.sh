#!/bin/bash
set -ev
#mysql
mysql -e "CREATE USER 'openrasp'@'%' IDENTIFIED BY '123456'";
mysql -e "USE mysql;UPDATE user SET authentication_string=PASSWORD('rasp#2019') WHERE user='root';FLUSH PRIVILEGES;";
ln -s /run/mysqld/mysqld.sock /tmp/mysql.sock

#pgsql
psql -c "ALTER USER postgres WITH PASSWORD 'postgres';" -U postgres ;
psql -c "CREATE USER openrasp WITH PASSWORD '123456'" -U postgres;

#mongo
mongo test --eval 'db.createUser({user:"openrasp",pwd:"rasp#2019",roles:["readWrite"]});'
mkdir -p /tmp/mongodb/rs0-0  /tmp/mongodb/rs0-1 /tmp/mongodb/ipv6
mongod --replSet rs0 --port 27018 --bind_ip localhost --dbpath /tmp/mongodb/rs0-0 --smallfiles --oplogSize 128 --fork --syslog
mongod --replSet rs0 --port 27019 --bind_ip localhost --dbpath /tmp/mongodb/rs0-1 --smallfiles --oplogSize 128 --fork --syslog
mongo --port 27018 --eval 'rs.initiate({_id: "rs0",members: [{_id: 0,host: "localhost:27018"},{_id: 1,host: "localhost:27019"}]})'

sudo sed -i 's/#module(load="imtcp")/module(load="imtcp")/g' /etc/rsyslog.conf
sudo sed -i 's/#input(type="imtcp" port="514")/input(type="imtcp" port="514")/g' /etc/rsyslog.conf
sudo service rsyslog restart
sudo service apache2 restart

ulimit -c
ulimit -a -S
ulimit -a -H
cat /proc/sys/kernel/core_pattern
cat /etc/default/apport

ulimit -c unlimited -S
sudo bash -c "echo '/tmp/coredump/core.%p.%E' > /proc/sys/kernel/core_pattern"

#mongo-php-driver
# git clone https://github.com/mongodb/mongo-php-driver.git
# pushd mongo-php-driver
# git submodule update --init
# phpize
# ./configure
# make all
# sudo make install
# echo "extension=mongodb.so" >> `php --ini | grep "Loaded Configuration" | sed -e "s|.*:\s*||"`
# popd
# pecl install mongodb
echo "extension = mongodb.so" >> ~/.phpenv/versions/$(phpenv version-name)/etc/php.ini


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