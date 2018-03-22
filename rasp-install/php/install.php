<?php
const UNKNOWN 		= "unknown";

const OS_WIN 		= "windows";
const OS_LINUX 		= "linux";
const OS_OSX 		= "mac";

const LINUX_UBUNTU 	= "ubuntu";
const LINUX_CENTOS 	= "centos";

const INFO 			= "INFO";
const DEBUG 		= "DEBUG";
const ERROR 		= "ERROR";

const UNFOUND 		= 0;
const FOUND 		= 1;
const FINSH 		= 2;


//获取linux发行版本
function get_linux_release_name() {
	if (file_exists("/etc/debian_version")) {
		return LINUX_UBUNTU;
	} else if(file_exists("/etc/redhat-release")) {
		return LINUX_CENTOS;
	} else {
		return UNKNOWN;
	}
}

//获取操作系统
function get_OS() {
	if (stristr(PHP_OS, 'DAR')) {
		return OS_OSX;
	} else if (stristr(PHP_OS, 'WIN')) {
		return OS_WIN;
	} else if (stristr(PHP_OS, 'LINUX')) {
		return OS_LINUX;
	} else {
		return UNKNOWN;
	}
}

//获取 ini_scanned_path
function get_ini_scanned_path() {
	$ini_scanned_files = php_ini_scanned_files();
	if ($ini_scanned_files) {
		$ini_scanned_arr = explode(",\n", trim(php_ini_scanned_files()));
		return dirname($ini_scanned_arr[0]);
	} else {
		return false;
	}
}

//安装流程展示
function major_tips($message, $done = FALSE) {
	global $index;
	echo <<<MSG
***********************************************************
 $index. $message
***********************************************************

MSG;
	$index++;
	if ($done) {
		exit(0);
	}
}

//日志
function log_tips($level, $msg, $arr = null) {
	echo '['.$level."]: ".$msg.PHP_EOL;
	if (!empty($arr) && is_array($arr)) {
		foreach ($arr as $key => $value) {
			echo $key." =>".$value.PHP_EOL;
		}
	}
	if ($level === ERROR) {
		exit(0); 	
	}
}

//帮助
function show_help() {
	echo <<<HELP
Synopsis:
    php install.php [options]

Options:
    -d <file>   <file> will act as OpenRASP work folder.

    -h          This Help.

HELP;
	exit(0); 
}

//全局变量
$index 				= 1;
$root_dir 			= null;
$current_os 		= get_OS();
$supported_sapi 	= array('apache2', 'cli', 'fpm');
$lib_filename 		= $current_os == OS_WIN ? 'openrasp.dll' : 'openrasp.so';
$lib_source_path 	= sprintf("%s%sphp%s%s-php%s.%s-%s%s%s", __DIR__, 
	DIRECTORY_SEPARATOR, DIRECTORY_SEPARATOR, 	$current_os, PHP_MAJOR_VERSION, 
	PHP_MINOR_VERSION, php_uname("m"), DIRECTORY_SEPARATOR, $lib_filename);
$extension_dir 		= ini_get('extension_dir');
$ini_loaded_file 	= php_ini_loaded_file();
$ini_scanned_path 	= get_ini_scanned_path();
$ini_scanned_file 	= 'openrasp.ini';

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 过程化安装 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
$options = getopt("d:h");
if (array_key_exists("d", $options) && !empty($options["d"])) {
	$root_dir = $options["d"];
	log_tips(INFO, "openrasp.root_dir => ".$root_dir);
} else if (array_key_exists("h", $options)) {
	show_help();
} else {
	log_tips(ERROR, "Fail to parse arguments, please use \"-h\" to check help information.");
}

major_tips('copy openrasp lib to extension_dir');
if (!file_exists($extension_dir)) {
	log_tips(ERROR, $extension_dir.' not exist!');
}
if (!is_writable($extension_dir)) {
	log_tips(ERROR, $extension_dir.' is not writable, please make sure you have write permissions!');
}
if (!file_exists($lib_source_path)) {
	log_tips(ERROR, 'unsupported system or php version.');
}
$lib_dest_path = $extension_dir.DIRECTORY_SEPARATOR.$lib_filename;
if (file_exists($lib_dest_path)
	&& !rename($lib_dest_path, $lib_dest_path.'.bak')) {
	log_tips(ERROR, $lib_dest_path.' backup failure!');
}
if (!copy($lib_source_path, $extension_dir.DIRECTORY_SEPARATOR.$lib_filename)) {
	log_tips(ERROR, 'fail to copy openrasp lib into extension_dir!');
} else {
	log_tips(INFO, 'Successfully copy '.$lib_filename.' to '.$extension_dir);
}

major_tips('modify php.ini');
$ini_content = <<<OPENRASP
;OPENRASP BEGIN

extension=openrasp.so
openrasp.root_dir="$root_dir"

;拦截攻击后，跳转到这个URL，并增加 request_id 参数
;openrasp.block_url=https://rasp.baidu.com/blocked/

;数组回调函数黑名单，命中即拦截
;openrasp.callable_blacklists=system,exec,passthru,proc_open,shell_exec,popen,pcntl_exec,assert

;当服务器不满足安全配置规范，是否禁止服务器启动
;openrasp.enforce_policy=Off

;hook 点黑名单，逗号分隔
;openrasp.hooks_ignore=

;对于以下URL，修改响应并注入HTML
;openrasp.inject_urlprefix=

;国际化配置
;openrasp.locale=

;每个进程/线程每秒钟最大日志条数
;openrasp.log_maxburst=1000

;当SQL查询结果行数大于或等于该值，则认为是慢查询
;openrasp.slowquery_min_rows=500

;报警是否开启 syslog
;openrasp.syslog_alarm_enable=Off

;用于 syslog 的 facility
;openrasp.syslog_facility=

;syslog 服务器地址
;openrasp.syslog_server_address=

;对于单个请求，JS插件整体超时时间（毫秒）
;openrasp.timeout_ms=100

;OPENRASP END


OPENRASP;

if ($ini_scanned_path) {
	$linux_release_name = UNKNOWN;
	$ini_system_links = null;
	if ($current_os == OS_LINUX) {
		$linux_release_name = get_linux_release_name();
	}
	if ($linux_release_name == LINUX_UBUNTU) {
		$ini_scanned_root = stristr($ini_scanned_path, 'cli', true);
		if ($ini_scanned_root) {
			$ini_scanned_path = $ini_scanned_root."mods-available";
			foreach ($supported_sapi as $key => $value) {
				if (file_exists($ini_scanned_root.$value) && is_dir($ini_scanned_root.$value)) {
					$ini_system_links[$value] = $ini_scanned_root.$value.DIRECTORY_SEPARATOR.'conf.d/99-openrasp.ini';	
				}
			}
		}
	}
	if (!is_writable($ini_scanned_path)) {
		log_tips(ERROR, $ini_scanned_path.' is not writable, please make sure you have write permissions!');
	}


	$handle = fopen($ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file, "w+");
	if ($handle) {
		if (fwrite($handle, $ini_content) === FALSE) {
			fclose($handle);
			log_tips(ERROR, 'Cannot write to '.$ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file);
		} else {
			log_tips(INFO, 'Successfully write openrasp config to '.$ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file);
		}
		fclose($handle);
		if (!empty($ini_system_links) && is_array($ini_system_links)) {
			log_tips(INFO, 'Found system links of openrasp.ini are:', $ini_system_links);
			foreach ($ini_system_links as $key => $value) {
				if (file_exists($value) && readlink($value) === $ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file) {
					continue;
				}
				if (!symlink($ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file, $value)) {
					log_tips(ERROR, 'Fail to create link '.$value);
				}
			}
		}
	} else {
		log_tips(ERROR, 'Cannot open '.$ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file);
	} 
} else if ($ini_loaded_file) {
	if (!is_writable($ini_loaded_file)) {
		log_tips(ERROR, $ini_loaded_file.' is not writable, please make sure you have write permissions!');
	}
	if (!copy($ini_loaded_file, $ini_loaded_file.'.bak')) {
		log_tips(ERROR, $ini_loaded_file.' backup failure!');
	}

	$old_ini_data = file($ini_loaded_file.'.bak');
	$tmp_ini_data = array();
	$found_openrasp = UNFOUND;
	foreach ($old_ini_data as $key => $line) {
		if (trim($line) == ";OPENRASP BEGIN") {
			$found_openrasp = FOUND;
		} 
		if (trim($line) == ";OPENRASP END") {
			$found_openrasp = FINSH;
		}
		if (FOUND !== $found_openrasp && trim($line) != ";OPENRASP END") {
			$tmp_ini_data[] = $line;
		}
	}
	if (FOUND === $found_openrasp) {
		log_tips(ERROR, 'Error occurs during positioning openrasp config tag in'.$ini_loaded_file);
	} else if (FINSH === $found_openrasp) {
		log_tips(INFO, 'Openrasp config tags found, they will be removed.');
	}
	$tmp_ini_data[] = $ini_content;
			$fp = fopen($ini_loaded_file, "a+");
		flock($fp, LOCK_EX);
		foreach($tmp_ini_data as $key => $line) {
     		fwrite($fp, $line);
 		}
		flock($fp, LOCK_UN);
		fclose($fp);

	$handle = fopen($ini_loaded_file, "w+");
	if ($handle) {
		$write_state = TRUE;
		foreach($tmp_ini_data as $key => $line) {
			if (fwrite($handle, $line) === FALSE) {
				$write_state = FALSE;
				break;
			}
 		}
 		if ($write_state === FALSE) {
 			fclose($handle);
 			log_tips(INFO, 'Fail write ini content to '.$ini_loaded_file.', we will restore the php.ini file.');
 			if (!copy($ini_loaded_file.'.bak', $ini_loaded_file)) {
				log_tips(ERROR, 'Fail to restore the php.ini file, you must manually restore php.ini.');
			}
 		} else {
 			log_tips(INFO, 'Successfully append openrasp config to '.$ini_loaded_file);
 		}
		fclose($handle);
	} else {
		log_tips(ERROR, 'Cannot open '.$ini_loaded_file);
	} 
} else {
	log_tips(ERROR, 'Cannot find appropriate php.ini file.');
}


major_tips('Finish the installation.', TRUE); 
?>
