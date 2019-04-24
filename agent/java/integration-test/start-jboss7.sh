#!/bin/bash

set +e

wget -N http://download.jboss.org/jbossas/7.1/jboss-as-7.1.1.Final/jboss-as-7.1.1.Final.zip

unzip jboss-as-7.1.1.Final.zip

export SERVER_HOME=$(pwd)/jboss-as-7.1.1.Final

echo "export SERVER_HOME=$SERVER_HOME" > /tmp/openrasp_java_server_home.sh

cp app.war ${SERVER_HOME}/standalone/deployments

java -jar RaspInstall.jar -install ${SERVER_HOME}

nohup sh ${SERVER_HOME}/bin/standalone.sh &

sleep 10