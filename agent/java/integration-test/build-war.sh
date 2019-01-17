#!/bin/bash

if [[ $SERVER == "springboot" ]]; then
    cp app/*.jsp springboot-jsp-master/src/main/webapp

    pushd springboot-jsp-master/

    mvn clean package

    popd
else
	jar cvf app.war -C app/ .
fi
