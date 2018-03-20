#!/bin/bash

set -ex
script_base="$(readlink -f $(dirname "$0"))"

# PHP 版本和架构
php_version=$(php -r 'echo PHP_MAJOR_VERSION, ".", PHP_MINOR_VERSION;')
php_arch=$(uname -m)
php_os=

case "$(uname -s)" in
    Linux)     
		php_os=linux
		;;
    Darwin)
		php_os=macos
        ;;
    *)
		echo Unsupported OS: $(uname -s)
		exit 1
		;;
esac

# 下载 libv8
curl https://rasp.baidu.com/download/libv8-5.9-"$php_os".tar.gz -o /tmp/libv8-5.9.tar.gz
tar -xf /tmp/libv8-5.9.tar.gz -C /tmp/

# 确定编译目录
output_base="$script_base/rasp-php-$(date +%Y-%m-%d)"
output_ext="$output_base/php/${php_os}-php${php_version}-${php_arch}"

# 编译
cd agent/php
phpize --clean
phpize
if [[ $php_os == "macos" ]]; then
	./configure --with-v8=/tmp/libv8-5.9-${php_os}/ --with-gettext=/usr/local/homebrew/opt/gettext -q
else
	./configure --with-v8=/tmp/libv8-5.9-${php_os}/ --with-gettext -q
fi

make

# 复制扩展
mkdir -p "$output_ext"
cp modules/openrasp.so "$output_ext"/
make distclean
phpize --clean

# 复制其他文件
mkdir -p "$output_base"/{conf,assets,logs,locale,plugins}
cp ../../plugins/official/plugin.js "$output_base"/plugins

# 打包
cd "$script_base"
tar --numeric-owner --group=900 --owner=900 -cjvf "$(basename "$output_base")".tar.bz2 "$output_base"




