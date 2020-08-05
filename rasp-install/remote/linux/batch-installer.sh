#!/bin/bash
#
# Copyright 2017-2019 Baidu Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

set -x

export ERROR_RASP_INSTALL=1
export ERROR_APP_SHUTDOWN=2
export ERROR_APP_RESTART=3
export ERROR_USER_PERMISSION=4

function do_install_java()
{
	job=$1
	   
	# 定位 RaspInstall.jar 安装包
	install_jar=(rasp-*/RaspInstall.jar)
    if [[ "$install_jar" == "rasp-*/RaspInstall.jar" ]]; then

cat << EOF    	
OpenRASP Java package missing, e.g rasp-2018-11-20/RaspInstall.jar
Please download and extract the binary package, e.g

wget https://packages.baidu.com/app/openrasp/rasp-java.tar.gz && tar -xvf rasp-java.tar.gz
EOF

    	exit
    else
    	install_jar=$(readlink -f "$install_jar")
    fi

	# 不限制 virtual memory / max memory size，防止 JVM 出错
	ulimit -m unlimited -v unlimited

    # 开始逐个安装
	echo Looking for Java processes ..

	for pid in $(pidof java)
	do
		java_path=$(readlink -f /proc/$pid/exe)
		java_home=$(strings /proc/$pid/environ | awk -F = '/JAVA_HOME/ {print $2}')
		java_version=$($java_path -version 2>&1 | awk -F'"' '/java version/ {print $2}')
		java_cmdline=$(cat /proc/$pid/cmdline | tr '\0' ' ')
		java_ctime=$(stat -c %Z /proc/$pid)

		tomcat_home=$(strings /proc/$pid/cmdline | awk -F = '/-Dcatalina.home/ {print $2}')
		tomcat_version=$(strings /proc/$pid/environ | awk -F = '/tomcat_ver/ {print $2}')
		tomcat_ports=$(netstat -ltpn | grep $pid/ | awk '{print $4}' | sed 's/0.0.0.0/127.0.0.1/' | tr '\n' ' ')
		tomcat_uid=$(stat -c %u /proc/$pid)
		tomcat_user=$(getent passwd $tomcat_uid | awk -F: '{print $1}')
		tomcat_url=
		javaagent_list=$(strings /proc/$pid/cmdline | grep -- -javaagent:)
		has_openrasp=$(strings /proc/$pid/cmdline | grep -- '-javaagent:.*/rasp/rasp.jar')
		is_uninstall=$(echo $job | grep -- -uninstall)

		# 枚举监听的端口，定位任意一个可用的 HTTP/HTTPS URL
		for url in $tomcat_ports
		do
			status=$(curl --max-time 10 -k -s -o /dev/null -w "%{http_code}" http://$url)
			if [[ $status != "000" ]]; then
				tomcat_url=$url
				break
			fi
		done

		# tomcat 6 使用 JRE_HOME，且无法从环境变量里获取版本号
		if [[ ! -z "$tomcat_home" ]] && [[ ! -z "$tomcat_user" ]]; then

			if [[ $(id -u) == 0 ]]; then
				tomcat_version=$(su - "$tomcat_user" -c "JAVA_HOME=$java_home JRE_HOME=$java_home bash ${tomcat_home}/bin/catalina.sh version | awk '/Server number/ {print \$3}'")
			else
				tomcat_version=$(JAVA_HOME=$java_home JRE_HOME=$java_home bash ${tomcat_home}/bin/catalina.sh version | awk '/Server number/ {print $3}')
			fi

		fi

cat << EOF

[INFO] Processing Java process (PID $pid)

Java Path:       $java_path
Java HOME:       $java_home
Java Version:    $java_version
Creation Time:   $java_ctime
Command Line:    $java_cmdline

Tomcat Home:     $tomcat_home
Tomcat User:     $tomcat_user (UID=$tomcat_uid)
Tomcat Version:  $tomcat_version
Tomcat Ports:    $tomcat_ports
Tomcat URL:      $tomcat_url
JavaAgent List:  $javaagent_list
Has OpenRASP:    $has_openrasp
EOF

		# 如果不是root，检查 UID 是否等于进程 UID
		if [[ $(id -u) != 0 ]] && [[ $tomcat_uid != $(id -u) ]]; then
			echo
			echo Permission denied: The script is not running as root, it would work for the current account only: UID=$(id -u)
			echo PID $pid belongs to UID $tomcat_uid, skipped
			continue
		fi

		# 检查参数
		if [[ -z "$java_path" ]] || [[ -z "$java_version" ]] || [[ -z "$tomcat_home" ]] || [[ -z "$tomcat_version" ]]
		then
			echo
			echo Unsupported Java application server: not a Tomcat server.
			echo Please report if you think this in error: https://github.com/baidu/openrasp
			continue
		fi

		# 检查 LDAP 等非本地认证的情况，这种情况下 getent 可能取不到值吧
		if [[ -z "$tomcat_user" ]]; then
			echo
			echo Unsupported Linux environment: unable to determine running user of Java server
			echo Please report if you think this in error: https://github.com/baidu/openrasp
			continue
		fi

		# 检查版本
		tomcat_major=${tomcat_version%%.*}
		if [[ $tomcat_major -lt 6 ]] || [[ $tomcat_major -gt 9 ]]; then
			echo
			echo Unsupported tomcat major version: $tomcat_major, only Tomcat 6.X - 8.X is supported.
			continue
		fi

		java_major=$(echo $java_version | awk -F. '{print $2}')
		if [[ $java_major -lt 6 ]] || [[ $java_major -gt 9 ]]; then
			echo
			echo Unsupported Java major version: $java_major, only Java 1.6-1.8 is supported.
			continue
		fi

		# 检查冲突
		if [[ ! -z $javaagent_list ]]; then
			if [[ $javaagent_list =~ "/rasp/rasp.jar" ]]; then
				echo
				echo OpenRASP already installed, doing upgrade/uninstall operation
			else
				echo
				echo Unable to process Java application server: Another -javaagent is already running, aborted.
				continue
			fi
		fi

		# 检查 tomcat 是否可用
		if [[ -z "$tomcat_url" ]]; then
			echo
			echo Unable to determine if tomcat is alive .. no working URL available
			continue
		fi

		# 如果是卸载，检查是不是安装了 openrasp
		if [[ ! -z $is_uninstall ]] && [[ -z $has_openrasp ]]; then
			echo
			echo OpenRASP is not installed for this Tomcat server, skipped
			continue
		fi

		# 开始安装
		echo
		echo '[INFO] Executing RaspInstall.jar'

		if [[ $(id -u) == 0 ]]; then
			# 切换账号，避免root写入之后，造成权限问题
			su - "$tomcat_user" -c "$java_path -jar $install_jar $job $tomcat_home"
			ret=$?
		else
			$java_path -jar $install_jar $job $tomcat_home
			ret=$?
		fi

		if [[ $ret -ne 0 ]]; then
			echo RaspInstall failed with error: $ret
			exit $ERROR_RASP_INSTALL
		fi

		# 尝试关闭应用服务器，最多等待 30s
		echo
		echo '[INFO] Shutting down Java server'
		"${tomcat_home}/bin/shutdown.sh"

		count=0
		while [[ $count -lt 30 ]]
        do
			count=$(( $count + 1))
			if [[ ! -d /proc/$pid ]]; then
				echo '[INFO] Java application server stopped "gracefully" ...'.
				break
			else
				echo Waiting for PID $pid to disappear ... $count seconds passed
				sleep 1
			fi
		done

		# 超时，检查 PID 是否还是以前的进程
		if [[ -d /proc/$pid ]]; then
			# 避免 PID 条件竞争
			new_ctime=$(stat -c %Z /proc/$pid)
			if [[ $new_ctime == $java_ctime ]]; then
				echo Timed out, killing PID $pid ..
				kill -9 $pid
			fi
		fi

		# 尝试重新启动; 可能会有 TIME_WAIT 问题
		echo
		echo '[INFO] Starting Java server'

		if [[ $(id -u) == 0 ]]; then
			su - $tomcat_user -c "JAVA_HOME=$java_home JRE_HOME=$java_home ${tomcat_home}/bin/startup.sh"
		else
			JAVA_HOME=$java_home JRE_HOME=$java_home "${tomcat_home}/bin/startup.sh"
		fi

		# 检查是否成功，最多等待 15s
		count=0
		success=0

		while [[ $count -lt 15 ]]
        do
        	count=$(( $count + 1))
        	status=$(curl -s -o /dev/null -w "%{http_code}" $tomcat_url)
			if [[ $status != "000" ]]; then
				success=1
				break
			else
				echo Waiting for $tomcat_url to be available ... $count seconds passed
				sleep 1
			fi
		done

		if [[ $success -ne 1 ]]; then
			echo '[ERROR] Tomcat failed to start ...'
			exit $ERROR_APP_RESTART
		else
			echo
			echo '[INFO] Success. Tomcat has become active :-)'
			curl -I $tomcat_url
		fi

	done
}

function do_install_php()
{
    echo Not implemented yet
}

function do_help()
{
cat << EOF
OpenRASP batch install/uninstall tool - Copyright 2017-2019 Baidu Inc

Usage:
	./batch.sh -i
	./batch.sh -u

Additional parameter
    -a  Value of app_id
    -b  Value of app_secret
    -c  Value of backend_url

    -l  Specify programming language, php/java etc. (Not implemented yet)
    -d  Be more verbose
    -h  You're reading this
EOF

exit
}

flag_install=
flag_uninstall=
flag_debug=
flag_lang=
flag_help=
flag_appid=
flag_appsecret=
flag_backendurl=

cd "$(dirname "$0")"

while getopts "hl:j:iua:b:c:" arg
do
	case "$arg" in
		l)
			flag_lang=$OPTARG
			;;
		d)
			flag_debug=1
			;;
		i)
			flag_install=1
			;;
		u)
			flag_uninstall=1
			;;
		a)
			flag_appid=$OPTARG
			;;
		b)
			flag_appsecret=$OPTARG
			;;
		c)
			flag_backendurl=$OPTARG
			;;
		*)
			do_help
			;;
	esac
done

# reject default parameter
if [[ $flag_appid == "APPID" ]]; then
	echo ERROR: bad app id: APPID
	exit
elif [[ $flag_appsecret == "APPSECRET" ]]; then
	echo ERROR: bad app secret: APPSECRET
	exit
fi

if [[ $(id -u) != "0" ]]; then
	echo
	echo Notice: Not running OpenRASP batch installer as ROOT, will install for current account only
fi

if [[ ! -z $flag_install ]]; then
	arg="-install"
	if [[ ! -z "$flag_appsecret" ]] && [[ ! -z "$flag_appid" ]] && [[ ! -z "$flag_backendurl" ]]; then
		arg="-appid $flag_appid -appsecret $flag_appsecret -backendurl $flag_backendurl -install"
	fi

	do_install_java "$arg"
elif [[ ! -z "$flag_uninstall" ]]; then
	do_install_java "-uninstall"
else
	do_help
fi

