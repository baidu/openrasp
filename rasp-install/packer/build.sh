#!/bin/bash
cd "$(dirname "$0")"

package=../../rasp-java.tar.gz 
url=$1

if [[ ! -f "$package" ]]; then
    echo OpenRASP java package missing. Execute build-java.sh in the git root folder
    exit
fi

if [[ -z "$url" ]]; then
cat << EOF
OpenRASP standalone installer creation tool

Usage:
   $0 http://packages.baidu-int.com:8066/collect.php
      
EOF
    exit
fi

rm -rf tmp
mkdir -p tmp

cp ../remote/linux/batch-installer.sh tmp/batch.sh
tar -xf "$package" -C tmp

(cd tmp && tar czf work.tar.gz batch.sh rasp-*/)
size=$(stat -c "%s" tmp/work.tar.gz)

(
    sed "s~TAR_SIZE~$size~; s~REPORT_URL~$url~;" wrapper.sh
    cat tmp/work.tar.gz
) > tmp/installer.sh

chmod +x tmp/installer.sh
echo Wrote tmp/installer.sh
