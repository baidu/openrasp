#!/bin/bash

set +e

wget -N http://download.jboss.org/jbossas/6.1/jboss-as-distribution-6.1.0.Final.zip

unzip jboss-as-distribution-6.1.0.Final.zip

SERVER_HOME=$(pwd)/jboss-6.1.0.Final

sed -i -e 's/<parameter><inject bean="BootstrapProfileFactory"/<parameter class="java.io.File"><inject bean="BootstrapProfileFactory"/' ${SERVER_HOME}/server/default/conf/bootstrap/profile.xml

cp app.war ${SERVER_HOME}/server/default/deploy/

cp -R rasp ${SERVER_HOME}/

chmod 777 ${SERVER_HOME}/rasp

export JAVA_OPTS="-javaagent:${SERVER_HOME}/rasp/rasp.jar -Dlog4j.rasp.configuration=file://${SERVER_HOME}/rasp/conf/rasp-log4j.xml ${JAVA_OPTS}"

nohup sh ${SERVER_HOME}/bin/run.sh &