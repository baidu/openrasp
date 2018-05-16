#!/bin/bash

set +e

pushd ../../../rasp-install/java

mvn clean package

cp target/RaspInstall.jar ../../agent/java/integration-test/RaspInstall.jar

popd
