#!/bin/bash

set +e

wget -N http://archive.apache.org/dist/tomcat/tomcat-7/v7.0.81/bin/apache-tomcat-7.0.81.tar.gz

tar zxf apache-tomcat-7.0.81.tar.gz

export SERVER_HOME=$(pwd)/apache-tomcat-7.0.81

echo "export SERVER_HOME=$SERVER_HOME" > /tmp/openrasp_java_server_home.sh

cp springboot-jsp-master/target/app.war ${SERVER_HOME}/webapps/

java -jar RaspInstall.jar -install ${SERVER_HOME}

sh ${SERVER_HOME}/bin/startup.sh