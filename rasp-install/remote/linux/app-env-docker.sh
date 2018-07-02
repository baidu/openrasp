#!/bin/bash
#
# Installation script for app-env-docker repository
# https://github.com/baidu-security/app-env-docker

set -x
cd /tmp

norestart=0

if [[ ! -z "$1" ]]; then
    norestart=1
fi

function InstallJava()
{
	path=$1

	curl https://packages.baidu.com/app/openrasp/rasp-java.tar.gz -o rasp-java.tar.gz
	tar -xvf rasp-java.tar.gz
	
	cd rasp-*
	java -jar RaspInstall.jar -install "$path"
	rm -rf rasp*
}

function InstallSpringBoot()
{
	curl https://packages.baidu.com/app/openrasp/rasp-java.tar.gz -o rasp-java.tar.gz
	tar -xvf rasp-java.tar.gz

	rm -rf /rasp
	mv rasp-*/rasp /rasp
}

function InstallPHP()
{
	curl https://packages.baidu.com/app/openrasp/rasp-php.tar.bz2 -o rasp-php.tar.bz2
	tar -xvf rasp-php.tar.bz2
	
	php rasp-*/install.php -d /opt/rasp/
	rm -rf rasp*
}

if [[ -d /tomcat/ ]]; then
	InstallJava /tomcat/

    if [[ "$norestart" != 1 ]]; then
	    /etc/init.d/tomcat.sh restart
    fi
fi

if [[ -d /jboss/ ]]; then
	InstallJava /jboss/
    
    if [[ "$norestart" != 1 ]]; then
	    /etc/init.d/jboss.sh restart
    fi
fi

if [[ -f /root/springboot.jar ]]; then
	InstallSpringBoot
fi

if [[ -x /usr/bin/php ]]; then
	InstallPHP
	/etc/init.d/httpd.sh restart
fi

