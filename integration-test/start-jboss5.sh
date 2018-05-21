#!/bin/bash

set +e

wget -N http://iweb.dl.sourceforge.net/project/jboss/JBoss/JBoss-5.1.0.GA/jboss-5.1.0.GA.zip

unzip jboss-5.1.0.GA.zip

export SERVER_HOME=$(pwd)/jboss-5.1.0.GA

sed -i -e 's/<parameter><inject bean="BootstrapProfileFactory"/<parameter class="java.io.File"><inject bean="BootstrapProfileFactory"/' ${SERVER_HOME}/server/default/conf/bootstrap/profile.xml

cp app.war ${SERVER_HOME}/server/default/deploy/

# cp -R rasp ${SERVER_HOME}/

# chmod 777 ${SERVER_HOME}/rasp

# export JAVA_OPTS="-javaagent:${SERVER_HOME}/rasp/rasp.jar -Dlog4j.rasp.configuration=file://${SERVER_HOME}/rasp/conf/rasp-log4j.xml ${JAVA_OPTS}"

# wget -N $(curl -s https://api.github.com/repos/baidu/openrasp/releases/latest | grep "browser_download_url.*rasp-java\.tar\.gz" | cut -d '"' -f 4)

# tar zxf rasp-java.tar.gz --strip-components=1  $(tar ztf rasp-java.tar.gz --wildcards "*RaspInstall.jar")

java -jar RaspInstall.jar -install ${SERVER_HOME}

nohup sh ${SERVER_HOME}/bin/run.sh &
