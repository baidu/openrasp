#!/bin/bash

set +e

wget -N http://caucho.com/download/resin-4.0.58.tar.gz

tar zxf resin-4.0.58.tar.gz

export SERVER_HOME=$(pwd)/resin-4.0.58

echo "export SERVER_HOME=$SERVER_HOME" > /tmp/openrasp_java_server_home.sh

cp app.war ${SERVER_HOME}/webapps/

java -jar RaspInstall.jar -install ${SERVER_HOME}

sh ${SERVER_HOME}/bin/resin.sh start