#!/bin/bash

set +e

pushd ../agent/java

mvn clean package

cp boot/target/rasp.jar ../../integration-test/rasp/rasp.jar
cp engine/target/rasp-engine.jar ../../integration-test/rasp/rasp-engine.jar

popd

pushd ../rasp-install/java

mvn clean package

cp target/RaspInstall.jar ../../integration-test/RaspInstall.jar

popd
