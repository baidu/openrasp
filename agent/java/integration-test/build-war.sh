#!/bin/bash

if [[ $SERVER == "springboot" ]]; then
    cp app/*.jsp springboot-jsp-master/src/main/webapp

    pushd springboot-jsp-master/

    mvn clean package

    popd
elif [[ $SERVER == "dubbo" ]]; then
	pushd dubbo-master/

	mvn clean package

	popd

	jar cvf app.war -C app/ .
else
	jar cvf app.war -C app/ .
fi
