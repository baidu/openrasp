#!/bin/bash
#
# Installation script for app-env-docker repository
# https://github.com/baidu-security/app-env-docker

set -x
cd /tmp

function InstallJava()
{
	path=$1

	curl https://packages.baidu.com/app/openrasp/rasp-java.tar.gz -O rasp-java.tar.gz
	tar -xvf rasp-java.tar.gz
	
	cd rasp-*
	java -jar RaspInstall.jar "$path"
	rm -rf rasp*
}

function InstallPHP()
{
	curl https://packages.baidu.com/app/openrasp/rasp-php.tar.bz2 -O rasp-php.tar.bz2
	tar -xvf rasp-php.tar.bz2
	
	php rasp-*/install.php -d /opt/rasp/
	rm -rf rasp*
}

if [[ -d /tomcat/ ]]; then
	InstallJava /tomcat/
	/etc/init.d/tomcat.sh restart
fi

if [[ -d /jboss/ ]]; then
	InstallJava /jboss/
	/etc/init.d/jboss.sh restart
fi

if [[ -x /usr/bin/php ]]; then
	InstallPHP
	/etc/init.d/httpd.sh restart
fi

