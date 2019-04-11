#!/bin/bash

set +e

wget -N https://download.jboss.org/wildfly/10.0.0.Final/wildfly-10.0.0.Final.zip

unzip wildfly-10.0.0.Final.zip

export SERVER_HOME=$(pwd)/wildfly-10.0.0.Final

echo "export SERVER_HOME=$SERVER_HOME" > /tmp/openrasp_java_server_home.sh

cp app.war ${SERVER_HOME}/standalone/deployments

java -jar RaspInstall.jar -install ${SERVER_HOME}

nohup sh ${SERVER_HOME}/bin/standalone.sh &