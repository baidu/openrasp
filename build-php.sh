#!/bin/bash
#
# To enable native antlr support, do
# 
# wget https://packages.baidu.com/app/openrasp/libantlr4-linux.tar.gz -O /tmp/libantlr4.tar.gz
# tar -xf /tmp/libantlr4.tar.gz -C /tmp
# extra_config_opt="--with-antlr4=/tmp/libantlr4" bash build-php.sh
# 
# 中文 PHP 扩展编译说明
# https://rasp.baidu.com/doc/hacking/compile/php.html

cd "$(dirname "$0")"

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
curl https://packages.baidu.com/app/openrasp/libv8-5.9-"$php_os".tar.gz -o /tmp/libv8-5.9.tar.gz
tar -xf /tmp/libv8-5.9.tar.gz -C /tmp/

# 确定编译目录
output_base="$script_base/rasp-php-$(date +%Y-%m-%d)"
output_ext="$output_base/php/${php_os}-php${php_version}-${php_arch}"

# 编译
cd agent/php
phpize --clean
phpize
if [[ $php_os == "macos" ]]; then
	./configure --with-v8=/tmp/libv8-5.9-${php_os}/ --with-gettext=/usr/local/homebrew/opt/gettext -q ${extra_config_opt}
else
	./configure --with-v8=/tmp/libv8-5.9-${php_os}/ --with-gettext -q ${extra_config_opt}
fi

make

# 复制扩展
mkdir -p "$output_ext"
cp modules/openrasp.so "$output_ext"/
make distclean
phpize --clean

# 复制其他文件
mkdir -p "$output_base"/{conf,assets,logs,locale,plugins}
cp ../../plugins/official/plugin.js "$output_base"/plugins/official.js
cp ../../rasp-install/php/*.php "$output_base"

# 生成并拷贝mo文件
./scripts/locale.sh
mv ./po/locale.tar "$output_base"/locale
cd "$output_base"/locale
tar xvf locale.tar && rm -f locale.tar

# 打包
cd "$script_base"
tar --numeric-owner --group=0 --owner=0 -cjvf "$script_base/rasp-php.tar.bz2" "$(basename "$output_base")"




