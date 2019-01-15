#!/bin/bash

if [[ $SERVER == "springboot" ]]; then
    pushd springboot-jsp-master/
    mvn clean package
    popd
else
	jar cvf app.war -C app/ .
fi
