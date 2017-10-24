#!/bin/bash

set +e

wget -N http://central.maven.org/maven2/org/eclipse/jetty/jetty-distribution/9.4.7.v20170914/jetty-distribution-9.4.7.v20170914.tar.gz

tar zxf jetty-distribution-9.4.7.v20170914.tar.gz

export SERVER_HOME=$(pwd)/jetty-distribution-9.4.7.v20170914

cp app.war ${SERVER_HOME}/webapps/

cp -R rasp ${SERVER_HOME}/

chmod 777 ${SERVER_HOME}/rasp

export JAVA_OPTS="-javaagent:${SERVER_HOME}/rasp/rasp.jar -Dlog4j.rasp.configuration=file://${SERVER_HOME}/rasp/conf/rasp-log4j.xml ${JAVA_OPTS}"

pushd ${SERVER_HOME}

nohup java ${JAVA_OPTS} -jar start.jar &

popd