#!/bin/bash

set +e

wget http://archive.apache.org/dist/tomcat/tomcat-6/v6.0.53/bin/apache-tomcat-6.0.53.tar.gz

tar zxf apache-tomcat-6.0.53.tar.gz

HOME=$(pwd)/apache-tomcat-6.0.53

cp app.war ${HOME}/webapps/

cp -R rasp ${HOME}/

chmod 777 ${HOME}/rasp

export JAVA_OPTS="-javaagent:${HOME}/rasp/rasp.jar -Dlog4j.rasp.configuration=file://${HOME}/rasp/conf/rasp-log4j.xml ${JAVA_OPTS}"

sh ${HOME}/bin/startup.sh