#!/bin/bash

set +e

wget http://iweb.dl.sourceforge.net/project/jboss/JBoss/JBoss-4.2.3.GA/jboss-4.2.3.GA.zip

unzip jboss-4.2.3.GA.zip

HOME=$(pwd)/jboss-4.2.3.GA

cp app.war ${HOME}/server/default/deploy/

cp -R rasp ${HOME}/

chmod 777 ${HOME}/rasp

export JAVA_OPTS="-javaagent:${HOME}/rasp/rasp.jar -Dlog4j.rasp.configuration=file://${HOME}/rasp/conf/rasp-log4j.xml ${JAVA_OPTS}"

nohup sh ${HOME}/bin/run.sh &