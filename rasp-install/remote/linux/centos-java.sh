#!/bin/bash

set -ex

wget https://packages.baidu.com/app/openrasp/rasp-java.tar.gz -O - | tar -xzvf -
cd rasp-*
java -jar RaspInstall.jar /tomcat/

