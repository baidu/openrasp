#!/bin/bash

set +e

pushd ../agent/java

mvn clean package

cp target/rasp.jar ../../integration-test/rasp/rasp.jar

popd