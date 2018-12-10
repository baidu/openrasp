#!/bin/bash
#
# Build our VUE frontend and golang api server
# Output to rasp-cloud.tar.gz
#

set -ex

function check_prerequisite()
{
    npm=$(which npm)
    go=$(which go)

    if [[ -z "$npm" ]]; then
        echo NodeJS is required to build VUE frontend
        exit
    fi

    if [[ -z "$go" ]]; then
        echo GO binary is required to build backend API server
        exit
    fi
}

function repack()
{
    tar=$1
    name=$2
    output=$3

    rm -rf tmp "$output"

    mkdir tmp
    tar xf "$tar" -C tmp

    mv tmp "$name"
    tar --numeric-owner --owner=0 --group=0 -czf "$output" "$name"
}

function build_cloud()
{
    cd cloud

    export GOPATH=$(pwd)
    export PATH=$PATH:$GOPATH/bin

    go get -u github.com/beego/bee

    cd src/rasp-cloud
    bee pack

    repack rasp-cloud.tar.gz rasp-cloud-$(date +%Y-%m-%d) ../../../rasp-cloud.tar.gz
}

function build_vue()
{
    cd rasp-vue
    npm install --unsafe-perm
    npm run build

    rm -rf ../cloud/src/rasp-cloud/dist
    mv dist ../cloud/src/rasp-cloud/
}

cd "$(dirname "$0")"

check_prerequisite
(build_vue)
(build_cloud)
