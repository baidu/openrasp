#!/bin/bash
set -ev

source /tmp/openrasp_java_server_home.sh
pushd agent/java/integration-test/tester
node_modules/.bin/mocha *.test.js -t 30000 --retries 3
popd
