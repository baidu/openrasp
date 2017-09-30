#!/bin/bash

set +e

wget http://central.maven.org/maven2/org/eclipse/jetty/jetty-distribution/9.4.7.v20170914/jetty-distribution-9.4.7.v20170914.tar.gz

tar zxf jetty-distribution-9.4.7.v20170914.tar.gz

HOME=$(pwd)/jetty-distribution-9.4.7.v20170914

cp app.war ${HOME}/webapps/

cp -R rasp ${HOME}/

chmod 777 ${HOME}/rasp

export JAVA_OPTS="-javaagent:${HOME}/rasp/rasp.jar -Dlog4j.rasp.configuration=file://${HOME}/rasp/conf/rasp-log4j.xml ${JAVA_OPTS}"

pushd ${HOME}

nohup java ${JAVA_OPTS} -jar start.jar &

popd