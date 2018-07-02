#!/bin/bash
set -ev

pushd agent/java
pushd integration-test
bash build-war.sh
bash build-rasp.sh
bash build-rasp-install.sh
rm -rf ~/.m2/repository/com/baidu
bash start-$SERVER.sh
pushd tester
yarn
popd
popd