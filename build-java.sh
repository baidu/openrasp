#!/bin/bash
#
# 手动编译说明
# https://rasp.baidu.com/doc/hacking/compile/java.html

##################################
#                                #
# 自动编译，并生成OpenRASP安装包 #   
#                                #
##################################

cd "$(dirname "$0")"

BASE_DIR=$(pwd)
echo "base dir: $BASE_DIR"

PLUGIN_ROOT=$BASE_DIR/plugins/official
OUTPUT_ROOT=$BASE_DIR/rasp-$(date +%Y-%m-%d)
BASENAME="$(basename $OUTPUT_ROOT)"

rm -rf "$OUTPUT_ROOT" rasp-java.{zip,tar.gz}
mkdir -p "$OUTPUT_ROOT"/rasp/{plugins,conf} || exit 1

function log {
	echo "================= $1 ==================="
}

function buildRaspInstall {
	cd $BASE_DIR/rasp-install/java
	log "mvn clean package..."
	mvn clean package || exit 1 
	cp $BASE_DIR/rasp-install/java/target/RaspInstall.jar $OUTPUT_ROOT || exit 1
	rm -rf $BASE_DIR/rasp-install/java/target
}

# 编译Rasp
function buildRasp {
	cd $BASE_DIR/agent/java || exit 1
	log "mvn clean package"
	mvn clean package  || exit 1
	cp $BASE_DIR/agent/java/boot/target/rasp.jar $OUTPUT_ROOT/rasp || exit 1
	cp $BASE_DIR/agent/java/engine/target/rasp-engine.jar $OUTPUT_ROOT/rasp || exit 1
}

function buildPlugin {
	cd $PLUGIN_ROOT || exit 1 
	cp $PLUGIN_ROOT/plugin.js $OUTPUT_ROOT/rasp/plugins/official.js || exit 1
}

function copyConf {
	cp $BASE_DIR/rasp-install/java/src/main/resources/openrasp.yml $OUTPUT_ROOT/rasp/conf/openrasp.yml || exit 1
}

log "[1] build RaspInstall.jar" 
buildRaspInstall

log "[2] copy OpenRASP Plugin"
buildPlugin

log "[3] copy rasp.yaml"
copyConf

log "[4] build OpenRASP"
buildRasp

cd $OUTPUT_ROOT/..
target=rasp-java.tar.gz
tar -czvf $target $BASENAME || exit
#mv $target $BASE_DIR || exit
log "Created $target"

target=rasp-java.zip
zip -r $target $BASENAME || exit
#mv $target $BASE_DIR || exit
log "Created $target"

log "SUCCESS!"

rm -rf $BASENAME


