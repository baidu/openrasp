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

    git_root=$(dirname $(readlink -f "$output"))
    rm -rf tmp "$output"

    mkdir tmp
    tar xf "$tar" -C tmp

    # 仅在 Linux 编译环境下 strip 二进制包
    if [[ $(uname -s) == "Linux" ]] && file tmp/rasp-cloud | grep -q 'ELF 64-bit'; then
        strip -s tmp/rasp-cloud
    fi

    # 安装默认插件
    mkdir tmp/resources
    rm -rf tmp/dist

    cp -R "$git_root"/plugins/official/plugin.js tmp/resources
    cp -R "$git_root"/rasp-vue/dist tmp

    mv tmp "$name"
    tar --numeric-owner --owner=0 --group=0 -czf "$output" "$name"

    # 删除临时文件，以前的包
    rm -rf "$name" "$tar"
}

function build_cloud()
{
    cd cloud

    export GOPATH=$(pwd)
    export PATH=$PATH:$GOPATH/bin

    if [[ -z "$NO_BEE_INSTALL" ]]; then
        go get -u github.com/beego/bee
    fi

    cd src/rasp-cloud

    commit=$(git rev-parse HEAD 2>/dev/null)
    build_time=$(date "+%Y-%m-%d %H:%M:%S" 2>/dev/null)
    if [[ $? -eq 0 ]]; then
        cat > tools/info.go << EOF
package tools

var CommitID = "${commit}"
var BuildTime = "${build_time}"
EOF
    fi

    # linux
    GOOS=linux bee pack -exr=vendor
    repack rasp-cloud.tar.gz rasp-cloud-$(date +%Y-%m-%d) ../../../rasp-cloud.tar.gz

    # mac
    GOOS=darwin bee pack -exr=vendor
    repack rasp-cloud.tar.gz rasp-cloud-$(date +%Y-%m-%d) ../../../rasp-cloud-mac.tar.gz
}

function build_vue()
{
    cd rasp-vue

    if [[ -z "$NO_NPM_INSTALL" ]]; then
        npm ci --unsafe-perm
    fi

    npm run build
}

cd "$(dirname "$0")"

check_prerequisite
(build_vue)
(build_cloud)

