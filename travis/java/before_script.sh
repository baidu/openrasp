#!/bin/bash
set -ev

bash build-java.sh
tar -xf rasp-java.tar.gz --strip 1 -C agent/java/integration-test
pushd agent/java/integration-test
bash build-war.sh
rm -rf ~/.m2/repository/com/baidu
rm rasp/plugins/official.js
bash start-$SERVER.sh
pushd tester
yarn
popd
popd