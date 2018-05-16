#!/bin/bash

set +e

pushd ..

mvn clean package

cp target/rasp.jar integration-test/rasp/rasp.jar

popd