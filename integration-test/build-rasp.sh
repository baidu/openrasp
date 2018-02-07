#!/bin/bash

set +e

pushd ../agent/java

mvn clean package

cp boot/target/rasp.jar ../../integration-test/rasp/rasp.jar
cp engine/target/rasp-engine.jar ../../integration-test/rasp/rasp-engine.jar

popd
