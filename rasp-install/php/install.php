<?php
/*
 * Copyright 2017-2018 Baidu Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
?>
OpenRASP Installer for PHP servers - Copyright 2017-2018 Baidu Inc.
For more details visit: https://rasp.baidu.com/doc/install/software.html

<?php
include_once(__DIR__ . DIRECTORY_SEPARATOR .'util.php');

//获取将要安装动态库绝对路径(get absolute path of lib to be installed)
function get_lib_2b_installed($current_os, $lib_filename)
{
	$machine_type_convertion = array(
		'i586'=>'x86',
		'AMD64'=>'x64'
	);
	$machine_type = php_uname('m');
	if (array_key_exists($machine_type, $machine_type_convertion)) {
		$machine_type = $machine_type_convertion[$machine_type];
	}
	$lib_abspath = sprintf("%s%sphp%s%s-php%s.%s-%s%s%s", __DIR__, 
	DIRECTORY_SEPARATOR, DIRECTORY_SEPARATOR, $current_os, PHP_MAJOR_VERSION, 
	PHP_MINOR_VERSION, $machine_type, DIRECTORY_SEPARATOR, $lib_filename);
	return $lib_abspath;
}

function check_dep_exts_installed($dep_exts)
{
	if (is_array($dep_exts)) {
		foreach($dep_exts as $key=>$value) {
			if (!extension_loaded($value)) {
				return false;
			}
		}
		return true;
	} else {
		return false;
	}
}

function get_ini_content($lib_filename, $root_dir)
{
$ini_content = <<<OPENRASP
;OPENRASP BEGIN
	
extension=$lib_filename
openrasp.root_dir=$root_dir
	
;拦截攻击后，跳转到这个URL，并增加 request_id 参数
;openrasp.block_url=https://rasp.baidu.com/blocked/

;拦截攻击后，将状态码设置为这个值
;openrasp.block_status_code=302
	
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

;syslog connection timeout(毫秒)
;openrasp.syslog_connection_timeout=50
	
;syslog read timeout(毫秒)
;openrasp.syslog_read_timeout=10

;syslog重连时间间隔(秒)
;openrasp.syslog_connection_retry_interval=300

;对于单个请求，JS插件整体超时时间（毫秒）
;openrasp.timeout_ms=100

;插件获取堆栈的最大深度
;openrasp.plugin_maxstack=100

;报警日志记录的最大堆栈深度
;openrasp.log_maxstack=10
	
;OPENRASP END
	
OPENRASP;
return $ini_content;
}

$lib_source_path = get_lib_2b_installed($current_os, $lib_filename);
$install_help_msg = <<<HELP
Synopsis:
    php install.php [options]

Options:
    -d <openrasp_root>  Specify OpenRASP installation folder, which is essential
	
    --ignore-ini        Do not update PHP ini entries

    --ignore-plugin     Do not update the official javascript plugin

    -h                  Show help messages

HELP;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 过程化安装 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
$shortopts = "d:h";
$longopts = array("ignore-ini", "ignore-plugin");
$options = getopt($shortopts, $longopts);
if (array_key_exists("d", $options) && !empty($options["d"])) {
	$root_dir = $options["d"];
	log_tips(INFO, "openrasp.root_dir => ".$root_dir);
} else if (array_key_exists("h", $options)) {
	show_help($install_help_msg);
} else {
	log_tips(ERROR, "Bad command line arguments. Please use \"-h\" to check help messages.");
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 检察依赖扩展 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
major_tips('Check whether dependent PHP extensions are installed');
$dep_exts = array('json', 'PDO');
if (!check_dep_exts_installed($dep_exts)) {
	log_tips(ERROR, implode(" ",$dep_exts) . " must be installed correctly.");
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 拷贝动态库 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
major_tips('Installing OpenRASP PHP Extension');
if (!file_exists($extension_dir)) {
	log_tips(ERROR, "Extension directory '$extension_dir' does not exist");
}
if (!is_writable($extension_dir)) {
	log_tips(ERROR, "Extension directory '$extension_dir' is not writable, make sure you have write permissions");
}
if (!file_exists($lib_source_path)) {
	log_tips(ERROR, "Unsupported system or php version: expecting '$lib_source_path' to be present.");
}
$lib_dest_path = $extension_dir.DIRECTORY_SEPARATOR.$lib_filename;
if (file_exists($lib_dest_path)
	&& !rename($lib_dest_path, $lib_dest_path.'.bak')) {
	log_tips(ERROR, "Unable to backup old openrasp extension: $lib_dest_path");
}
if (!copy($lib_source_path, $lib_dest_path)) {
	log_tips(ERROR, "Failed to copy '$lib_filename' to '$lib_dest_path'");
} else {
	log_tips(INFO, "Successfully copied '$lib_filename' to '$extension_dir'");
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 更新ini配置 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
if (extension_loaded('openrasp') && array_key_exists("ignore-ini", $options)) {
	major_tips("Skipped update of php.ini since '--ignore-ini' is set");
} else {
	major_tips('Updating php.ini');
	if ($ini_scanned_path) {
		$ini_symbol_links = null;
		if ($current_os == OS_LINUX && LINUX_UBUNTU == get_linux_release_name()) {
			$ini_scanned_root = stristr($ini_scanned_path, 'cli', true);
			if ($ini_scanned_root) {
				$ini_scanned_path = $ini_scanned_root . "mods-available";
				foreach ($supported_sapi as $key => $value) {
					if (file_exists($ini_scanned_root.$value) && is_dir($ini_scanned_root.$value)) {
						$ini_symbol_links[$value] = $ini_scanned_root.$value.DIRECTORY_SEPARATOR . 'conf.d/99-openrasp.ini';	
					}
				}
			}
		}
		if (!is_writable($ini_scanned_path)) {
			log_tips(ERROR, $ini_scanned_path . ' is not writable, please make sure you have write permissions!');
		}
		
		$ini_src = $ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file;
		$handle  = fopen($ini_src, "w+");
		if ($handle) {
			if (fwrite($handle, get_ini_content($lib_filename, $root_dir)) === FALSE) {
				fclose($handle);
				log_tips(ERROR, 'Cannot write to '. $ini_src);
			} else {
				log_tips(INFO, "Successfully write openrasp config to '$ini_src'");
			}
			fclose($handle);
			if (!empty($ini_symbol_links) && is_array($ini_symbol_links)) {
				log_tips(INFO, "Detected symbol links of openrasp.ini:", $ini_symbol_links);
				foreach ($ini_symbol_links as $key => $value) {
					if (file_exists($value) && readlink($value) === $ini_src) {
						continue;
					}
					if (!symlink($ini_src, $value)) {
						log_tips(ERROR, "Unable to create symbol link '$ini_src' to '$value'");
					}
				}
			}
		} else {
			log_tips(ERROR, 'Unable to open ini file for writing: ' . $ini_src);
		} 
	} else if ($ini_loaded_file) {
		$ini_files_2b_updated = array($ini_loaded_file);
		if (OS_WIN == $current_os) {
			$wamp_apache_ini = dirname($ini_loaded_file).DIRECTORY_SEPARATOR.'phpForApache.ini';
			if (file_exists($wamp_apache_ini)) {
				array_push($ini_files_2b_updated, $wamp_apache_ini);
			}
		}
		foreach($ini_files_2b_updated as $key=>$ini_file) {
			if (!is_writable($ini_file)) {
				log_tips(ERROR, $ini_file . ' is not writable, please make sure you have write permissions!');
			}
			if (!copy($ini_file, $ini_file.'.bak')) {
				log_tips(ERROR, "Unable to backup old ini file: '$ini_file'");
			}
		
			$old_ini_data = file($ini_file.'.bak');
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
				log_tips(ERROR, "Unable to locate OPENRASP closing tags in '$ini_file', possibly corrupted ini file");
			} else if (FINSH === $found_openrasp) {
				log_tips(INFO, 'Found old configuration in INI files, doing upgrades');
			}
			$tmp_ini_data[] = get_ini_content($lib_filename, $root_dir);
			$handle = fopen($ini_file, "w+");
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
					 log_tips(INFO, 'Fail write ini content to '.$ini_file.', we will restore the php.ini file.');
					 if (!copy($ini_file.'.bak', $ini_file)) {
						log_tips(ERROR, 'Fail to restore the php.ini file, you must manually restore php.ini.');
					}
				 } else {
					 log_tips(INFO, 'Successfully append openrasp config to '.$ini_file);
				 }
				fclose($handle);
			} else {
				log_tips(ERROR, "Unable to open '$ini_file' for writing");
			}
		} 
	} else {
		log_tips(ERROR, 'Cannot find appropriate php.ini file.');
	}
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 初始化工作目录 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
major_tips('Initializing OpenRASP root folder (openrasp.root_dir)');
if (file_exists($root_dir)) {
	if (!chmod($root_dir, 0777)) {
		log_tips(ERROR, 'Fail to chmod '.$root_dir);
	}
} else {
	if (!mkdir($root_dir, 0777, TRUE)) {
		log_tips(ERROR, 'Unable to create directory: ' . $root_dir);
	}
}
foreach($openrasp_work_sub_folders as $key => $value) {
	$sub_item = realpath($root_dir).DIRECTORY_SEPARATOR.$key;
	if (file_exists($sub_item)) {
		if (substr(sprintf('%o', fileperms($sub_item)), -4) != strval($value)) {
			chmod($sub_item, $value);
		}
		if ($key === "plugins") {
			if (array_key_exists("ignore-plugin", $options)) {
				major_tips("Skipped update of the official javascript plugin since '--ignore-plugin' is set");
			} else {
				major_tips('Updating the official javascript plugin');
				$plugin_source_dir = __DIR__.DIRECTORY_SEPARATOR.$key;
				if (file_exists($plugin_source_dir)) {
					$official_plugins = scandir($plugin_source_dir);
					foreach($official_plugins as $key=>$plugin) {
						if ($plugin === '.'
						|| $plugin === '..'
						|| !is_file($plugin_source_dir.DIRECTORY_SEPARATOR.$plugin)
						|| !endsWith($plugin, '.js')) {
							continue;
						}
						if (file_exists($sub_item.DIRECTORY_SEPARATOR.$plugin)) {
							if (md5_file($plugin_source_dir.DIRECTORY_SEPARATOR.$plugin) === md5_file($sub_item.DIRECTORY_SEPARATOR.$plugin)) {
								log_tips(INFO, 'Skipping update of ' . $plugin . ' since no changes is detected');
								continue;
							}
							if (!rename($sub_item.DIRECTORY_SEPARATOR.$plugin, $sub_item.DIRECTORY_SEPARATOR.$plugin.'.bak')) {
								log_tips(ERROR, $sub_item.DIRECTORY_SEPARATOR.$plugin.' backup failure!');
							}
						}
						if (!copy($plugin_source_dir.DIRECTORY_SEPARATOR.$plugin, $sub_item.DIRECTORY_SEPARATOR.$plugin)) {
							log_tips(ERROR, 'Unable to update the official javascript plugin');
						} else {
							log_tips(INFO, 'Successfully update official plugin: '.$plugin);
						}
					}

				}
			}
		} else if ($key === "locale") {
			if (file_exists(__DIR__.DIRECTORY_SEPARATOR.$key)) {
				clear_dir($sub_item);
				recurse_copy(__DIR__.DIRECTORY_SEPARATOR.$key, $sub_item);
			}
		}
	} else {
		$old_mask = umask(0);
		$mkdir_res = mkdir($sub_item, $value);
		$old_mask = umask(0);
		if (!$mkdir_res) {
			log_tips(ERROR, "Unable to create directory: $sub_item");
		}
		if (file_exists(__DIR__.DIRECTORY_SEPARATOR.$key)) {
			recurse_copy(__DIR__.DIRECTORY_SEPARATOR.$key, $sub_item);
		}
	}
}

major_tips("Installation completed without errors, please restart PHP server to take effect.", TRUE); 
?>
