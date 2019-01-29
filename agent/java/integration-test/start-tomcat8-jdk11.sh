#!/bin/bash

set +e

wget -N http://archive.apache.org/dist/tomcat/tomcat-8/v8.5.21/bin/apache-tomcat-8.5.21.tar.gz

tar zxf apache-tomcat-8.5.21.tar.gz

export SERVER_HOME=$(pwd)/apache-tomcat-8.5.21

echo "export SERVER_HOME=$SERVER_HOME" > /tmp/openrasp_java_server_home.sh

cp app.war ${SERVER_HOME}/webapps/

java -jar RaspInstall.jar -install ${SERVER_HOME}

sh ${SERVER_HOME}/bin/startup.sh