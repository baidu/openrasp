#!/bin/bash
#
# To build the OpenRASP cloud control
# 
# The output file is rasp-cloud.tar.gz
#
set -e

function log {
	echo "================= $1 ==================="
}

function initPath {
	pushd ../../
        export GOPATH="$(pwd)"
        log "set \$GOPATH=$GOPATH"
        export PATH="$GOPATH/bin:$PATH"
        log "set \$PATH=$PATH"
	popd
}

function buildRaspCloud {
	go get -u github.com/beego/bee
    bee pack
    rm -f ../../../rasp-cloud.tar.gz
    cp rasp-cloud.tar.gz ../../../
    log "build complete"
}

cd "$(dirname "$0")/cloud/src/rasp-cloud"
base_dir=$(pwd)
log "base_dir: $base_dir"
initPath
buildRaspCloud







