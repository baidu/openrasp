#/bin/bash

set +e

killall java

bash start-$SERVER.sh

tester/node_modules/.bin/mocha tester/test.js -t 20000