#!/bin/bash

set +e

wget http://archive.apache.org/dist/tomcat/tomcat-8/v8.5.21/bin/apache-tomcat-8.5.21.tar.gz

tar zxf apache-tomcat-8.5.21.tar.gz

HOME=$(pwd)/apache-tomcat-8.5.21

cp app.war ${HOME}/webapps/

cp -R rasp ${HOME}/

chmod 777 ${HOME}/rasp

export JAVA_OPTS="-javaagent:${HOME}/rasp/rasp.jar -Dlog4j.rasp.configuration=file://${HOME}/rasp/conf/rasp-log4j.xml ${JAVA_OPTS}"

sh ${HOME}/bin/startup.sh