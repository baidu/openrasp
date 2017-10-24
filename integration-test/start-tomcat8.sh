#!/bin/bash

set +e

wget -N http://archive.apache.org/dist/tomcat/tomcat-8/v8.5.21/bin/apache-tomcat-8.5.21.tar.gz

tar zxf apache-tomcat-8.5.21.tar.gz

export SERVER_HOME=$(pwd)/apache-tomcat-8.5.21

cp app.war ${SERVER_HOME}/webapps/

cp -R rasp ${SERVER_HOME}/

chmod 777 ${SERVER_HOME}/rasp

export JAVA_OPTS="-javaagent:${SERVER_HOME}/rasp/rasp.jar -Dlog4j.rasp.configuration=file://${SERVER_HOME}/rasp/conf/rasp-log4j.xml ${JAVA_OPTS}"

sh ${SERVER_HOME}/bin/startup.sh