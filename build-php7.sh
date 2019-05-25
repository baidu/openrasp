#!/bin/bash
# 中文 PHP 扩展编译说明
# https://rasp.baidu.com/doc/hacking/compile/php.html

set -ex
script_base="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$script_base"

# PHP 版本和架构
php_version=$(php -r 'echo PHP_MAJOR_VERSION, ".", PHP_MINOR_VERSION;')
php_arch=$(uname -m)
php_zts=$(php -r 'echo ZEND_THREAD_SAFE ? "-ts" : "";')
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

# 编译 openrasp-v8
git submodule update --init --recursive
mkdir -p openrasp-v8/php/build
cd openrasp-v8/php/build
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo ..
make

cd "$script_base"

# 确定编译目录
output_base="$script_base/rasp-php-$(date +%Y-%m-%d)"
output_ext="$output_base/php${php_zts}/${php_os}-php${php_version}-${php_arch}"

# 编译
cd agent/php7
phpize --clean
phpize

if [[ $php_os == "macos" ]]; then
	./configure --with-openrasp-v8=${script_base}/openrasp-v8/ --with-gettext=/usr/local/homebrew/opt/gettext -q ${extra_config_opt}
else
	curl https://packages.baidu.com/app/openrasp/static-lib.tar.bz2 -o /tmp/static-lib.tar.bz2
	tar -xf /tmp/static-lib.tar.bz2 -C /tmp/

	./configure --with-openrasp-v8=${script_base}/openrasp-v8/ --with-gettext --enable-openrasp-remote-manager \
		--with-curl=/tmp/static-lib --with-openssl=/tmp/static-lib -q ${extra_config_opt}
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
cp ../../rasp-install/php/openrasp.yml "$output_base"/conf/openrasp.yml

# 生成并拷贝mo文件
./scripts/locale.sh
mv ./po/locale.tar "$output_base"/locale
cd "$output_base"/locale
tar xvf locale.tar && rm -f locale.tar

# 打包
cd "$script_base"
if [[ -z "$NO_TAR" ]]; then
	tar --numeric-owner --group=0 --owner=0 -cjvf "$script_base/rasp-php.tar.bz2" "$(basename "$output_base")"
fi




