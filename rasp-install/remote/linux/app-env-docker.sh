#!/bin/bash
#
# Installation script for app-env-docker repository
# https://github.com/baidu-security/app-env-docker

set -x
cd /tmp

function InstallJava()
{
	path=$1

	wget https://packages.baidu.com/app/openrasp/rasp-java.tar.gz
	tar -xvf rasp-java.tar.gz
	
	cd rasp-*
	java -jar RaspInstall.jar "$path"
	rm -rf rasp*
}

function InstallPHP()
{
	wget https://packages.baidu.com/app/openrasp/rasp-php.tar.bz2
	tar -xvf rasp-php.tar.bz2
	
	php rasp-*/install.php -d /opt/rasp/
	rm -rf rasp*
}

if [[ -d /tomcat/ ]]; then
	InstallJava /tomcat/
	bash /tomcat/bin/shutdown.sh
	bash /tomcat/bin/startup.sh
fi

if [[ -d /jboss/ ]]; then
	InstallJava /jboss/
fi

if [[ -x /usr/bin/php ]]; then
	InstallPHP
	apachectl -k restart
fi

