#!/bin/bash
#
# 批量安装流程
# 1. 尝试安装
# 2. 如果失败则进入卸载流程
# 3. 上传所有日志到指定的服务器

job=$1
if [[ "$job" != "-i" ]] && [[ "$job" != "-u" ]]; then
	echo "Usage: $0 [-i | -u] [-a appid -b appsecret -c backendurl]"
	exit
fi

# 自解压
tmp=$(mktemp -d)
self="$(readlink -f "$0")"
report_url=REPORT_URL

if [[ -z "$tmp" ]]; then
	echo mktemp ERROR: Unexpected empty result
	exit 1
fi

trap 'rm -rf $tmp' EXIT

cd "$tmp"
tail -c TAR_SIZE "$self" | tar -xzf -

# 保证root写入的文件，其他账号可读，e.g umask
chmod 777 -R "$tmp"

if [[ "$job" == "-i" ]]; then
	echo Installing OpenRASP
	echo Logging to install.log

	bash batch.sh "$@" &> install.log
	ret1=$?

	# 安装失败，执行卸载
	if [[ $ret1 -ne 0 ]]; then
		echo Rollback
		bash batch.sh -u &> uninstall.log
		ret2=$?
	else
		ret2=0
		touch uninstall.log
	fi
elif [[ "$job" == "-u" ]]; then
	echo Uninstall OpenRASP
	echo Logging to uninstall.log

	touch install.log
	bash batch.sh -u &> uninstall.log
else
	echo Unknown job $job
	exit 1
fi

cat install.log
cat uninstall.log

# 上传所有日志和状态代码
if [[ "$report_url" != "disabled" ]]; then
	echo Uploading logs
	curl $report_url --connect-timeout 20 --max-time 60 \
		-F job=$job \
		-F hostname=$(hostname) \
		-F install_return=$ret1 -F install_log=@install.log \
		-F uninstall_return=$ret2 -F uninstall_log=@uninstall.log
else
	echo report_url not set, skipped
fi

exit

