<?php
/*
 * Copyright 2017-2019 Baidu Inc.
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
OpenRASP Installer for PHP servers - Copyright 2017-2019 Baidu Inc.
For more details visit: https://rasp.baidu.com/doc/install/software.html

<?php
error_reporting(E_ALL);
$open_basedir = ini_get('open_basedir');
if (! empty ($open_basedir))
{
	echo "WARNING: open_basedir is configured and might affect the installation process";
	echo "         current value: $open_basedir\n";
}

foreach (array('/sys/fs/selinux/enforce', '/selinux/enforce') as $selinux)
{
	if (! file_exists($selinux))
	{
		continue;
	}

	if (@file_get_contents($selinux) == "1")
	{
		echo "ERROR: selinux is enabled, try disable it with 'setenforce 0'\n";
		exit;
	}
}

if (PHP_VERSION_ID < 50300) 
{
	echo sprintf("Error: OpenRASP works on PHP 5.3 and onwards, version %s.%s is not supported\n", PHP_MAJOR_VERSION, PHP_MINOR_VERSION);
	exit;
}
include_once(__DIR__ . DIRECTORY_SEPARATOR .'util.php');

//获取将要安装动态库绝对路径(get absolute path of lib to be installed)
function get_lib_2b_installed($current_os, $lib_filename)
{
	$machine_type_convertion = array(
		'i586' => 'x86',
		'AMD64' => 'x64'
	);
	$machine_type = php_uname('m');
	if (array_key_exists($machine_type, $machine_type_convertion)) {
		$machine_type = $machine_type_convertion[$machine_type];
	}
	$zts_suffix = ZEND_THREAD_SAFE ? "-ts" : "";
	$lib_abspath = sprintf("%s%sphp%s%s%s-php%s.%s-%s%s%s", __DIR__,
		DIRECTORY_SEPARATOR, $zts_suffix, DIRECTORY_SEPARATOR, $current_os, PHP_MAJOR_VERSION,
		PHP_MINOR_VERSION, $machine_type, DIRECTORY_SEPARATOR, $lib_filename);
	return $lib_abspath;
}

function check_dep_exts_installed($dep_exts)
{
	if (is_array($dep_exts)) {
		foreach ($dep_exts as $key => $value) {
			if (!extension_loaded($value)) {
				return false;
			}
		}
		return true;
	} else {
		return false;
	}
}

function update_file_if_need($src, $dest, $description = "")
{
	if (file_exists($dest)) {
		if (md5_file($src) === md5_file($dest)) {
			log_tips(INFO, 'Skipping update of ' . $description . ' since no changes is detected');
			return;
		}
		if (!rename($dest, $dest . '.bak')) {
			log_tips(ERROR, $dest . ' backup failure!');
		}
	}
	if (!copy($src, $dest)) {
		log_tips(ERROR, 'Unable to update the ' . $description);
	} else {
		log_tips(INFO, 'Successfully update ' . $description . ': ' . $dest);
	}
}

function get_ini_content($lib_filename, $root_dir, $remote_enable, $backend_url, $app_id, $app_secret)
{
$ini_content = <<<OPENRASP
;OPENRASP BEGIN
	
extension=$lib_filename
openrasp.root_dir=$root_dir
	
;国际化配置
;openrasp.locale=

;云端地址
openrasp.backend_url=$backend_url

;agent app_id
openrasp.app_id=$app_id

;agent secret
openrasp.app_secret=$app_secret

;远程管理开关
openrasp.remote_management_enable=$remote_enable

;心跳时间间隔
openrasp.heartbeat_interval=180
	
;OPENRASP END

OPENRASP;
return $ini_content;
}

$lib_source_path = get_lib_2b_installed($current_os, $lib_filename);
$install_help_msg = <<<HELP
Synopsis:
    php install.php [options]

Options:
    -d <openrasp_root>      Specify OpenRASP installation folder (required)

    --backend-url <url>     Value of backend_url (required for remote management)

    --app-id <id>           Value of app_id (required for remote management)

    --app-secret <secret>   Value of app_secret (required for remote management)

    --keep-ini              Do not update PHP ini entries

    --keep-plugin           Do not update the official javascript plugin (higher priority than --without-plugin)

    --keep-conf             Do not update the openrasp config in root_dir/conf directory

    --without-plugin        Do not install the official javascript plugin

    -h, --help              Show help messages

HELP;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 参数解析 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
$shortopts = "d:h";
$longopts = array("without-plugin", "keep-ini", "keep-plugin", "keep-conf", "app-id:", "app-secret:", "backend-url:", "help");
$options = getopt($shortopts, $longopts);
if (array_key_exists("h", $options) || array_key_exists("help", $options)) {
	show_help($install_help_msg);
} 
if (array_key_exists("d", $options) && !empty($options["d"])) {
	// 创建目录
	if (! file_exists($options["d"])) {
    	mkdir($options["d"], 0777, true);
	}

	$root_dir = realpath($options["d"]);
	if (empty ($root_dir)) {
		log_tips(ERROR, "Can't resolve realpath of " . $options["d"] . ": No such directory.");
	}
	log_tips(INFO, "openrasp.root_dir => ".$root_dir);
} else {
	log_tips(ERROR, "openrasp.root_dir must be specified via option \"-d\"");
}

$remote_enable = "0";
$backend_url = "";
$app_id = "";
$app_secret = "";
if (array_key_exists("backend-url", $options)) {
	if (!empty($options["backend-url"])) {
		if (parse_url($options["backend-url"])) {
			$backend_url = $options["backend-url"];
		} else {
			log_tips(ERROR, "backend-url option is an illegal URL.");
		}
	} else {
		log_tips(ERROR, "backend-url option cannot be empty.");
	}
}

if (array_key_exists("app-id", $options)) {
	if (!empty($options["app-id"]))
	{
		if (preg_match("/^[0-9a-fA-F]{40}$/", $options["app-id"]) != 0) {
			$app_id = $options["app-id"];
		} else {
			log_tips(ERROR, "app-id option format is incorrect.");
		}
	} else {
		log_tips(ERROR, "app-id option cannot be empty.");
	}
}

if (array_key_exists("app-secret", $options)) {
	if (!empty($options["app-secret"])) {
		if (preg_match("/^[0-9a-zA-Z_-]{43,45}/", $options["app-secret"]) != 0) {
			$app_secret = $options["app-secret"];
		} else {
			log_tips(ERROR, "app-secret option format is incorrect.");
		}
	} else {
		log_tips(ERROR, "app-secret option cannot be empty.");
	}
}

if (!empty($backend_url) && !empty($app_id) && !empty($app_secret)) {
	$remote_enable = "1";
} else if (!empty($backend_url) || !empty($app_id) || !empty($app_secret)) {
	log_tips(ERROR, "backend-url app-id app-secret options must be specified simultaneously.");
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 检查依赖扩展 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
major_tips('Check whether required PHP extensions are installed');
$dep_exts = array('json', 'PDO');
if (!check_dep_exts_installed($dep_exts)) {
	log_tips(ERROR, "OpenRASP depends on the following PHP extension: " . implode(" ", $dep_exts) . " . Make sure they are installed on your system.");
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 拷贝动态库 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
major_tips('Installing OpenRASP PHP extension');
if (!file_exists($extension_dir)) {
	log_tips(ERROR, "Extension directory '$extension_dir' does not exist");
}
if (!is_writable($extension_dir)) {
	log_tips(ERROR, "Extension directory '$extension_dir' is not writable, make sure you have write permissions");
}
if (!file_exists($lib_source_path)) {
	log_tips(ERROR, "Unsupported system or php: " . phpversion() . "\nUname: " . php_uname() .
		"\nExpecting '$lib_source_path' to be present." .
		"\nPlease check your system, php version and ZTS state.");
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
if (extension_loaded('openrasp') && array_key_exists("keep-ini", $options)) {
	major_tips("Skipped update of php.ini since '--keep-ini' is set");
} else {
	major_tips('Updating php.ini');
	if ($ini_scanned_path) {
		$ini_symbol_links = null;
		if ($current_os == OS_LINUX && LINUX_UBUNTU == get_linux_release_name()) {
			$ini_scanned_root = stristr($ini_scanned_path, 'cli', true);
			if ($ini_scanned_root) {
				$ini_scanned_path = $ini_scanned_root . "mods-available";
				foreach ($supported_sapi as $key => $value) {
					if (file_exists($ini_scanned_root . $value) && is_dir($ini_scanned_root . $value)) {
						$ini_symbol_links[$value] = $ini_scanned_root . $value . DIRECTORY_SEPARATOR . 'conf.d/99-openrasp.ini';
					}
				}
			}
		}
		if (!is_writable($ini_scanned_path)) {
			log_tips(ERROR, $ini_scanned_path . ' is not writable, make sure you have write permissions.');
		}

		$ini_src = $ini_scanned_path . DIRECTORY_SEPARATOR . $ini_scanned_file;
		$handle = fopen($ini_src, "w+");
		if ($handle) {
			if (fwrite($handle, get_ini_content($lib_filename, $root_dir, $remote_enable, $backend_url, $app_id, $app_secret)) === FALSE) {
				fclose($handle);
				log_tips(ERROR, 'Cannot write to ' . $ini_src);
			} else {
				log_tips(INFO, "Successfully write openrasp config to '$ini_src'");
			}
			fclose($handle);
			if (!empty($ini_symbol_links) && is_array($ini_symbol_links)) {
				log_tips(INFO, "Detected symbol links of openrasp.ini:", $ini_symbol_links);
				foreach ($ini_symbol_links as $key => $value) {
					if (!file_exists(dirname($value))) {
						log_tips(INFO, "Cuz of parent dir not exist, skip create symbol links: '$value'");
						continue;
					}
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
			$wamp_apache_ini = dirname($ini_loaded_file) . DIRECTORY_SEPARATOR . 'phpForApache.ini';
			if (file_exists($wamp_apache_ini)) {
				array_push($ini_files_2b_updated, $wamp_apache_ini);
			}
		}
		foreach ($ini_files_2b_updated as $key => $ini_file) {
			if (!is_writable($ini_file)) {
				log_tips(ERROR, $ini_file . ' is not writable, make sure you have write permissions');
			}
			if (!copy($ini_file, $ini_file . '.bak')) {
				log_tips(ERROR, "Unable to backup old ini file: '$ini_file'");
			}

			$old_ini_data = file($ini_file . '.bak');
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
			$tmp_ini_data[] = get_ini_content($lib_filename, $root_dir, $remote_enable, $backend_url, $app_id, $app_secret);
			$handle = fopen($ini_file, "w+");
			if ($handle) {
				$write_state = TRUE;
				foreach ($tmp_ini_data as $key => $line) {
					if (fwrite($handle, $line) === FALSE) {
						$write_state = FALSE;
						break;
					}
				}
				if ($write_state === FALSE) {
					fclose($handle);
					log_tips(INFO, 'Fail write ini content to ' . $ini_file . ', we will restore the php.ini file.');
					if (!copy($ini_file . '.bak', $ini_file)) {
						log_tips(ERROR, 'Fail to restore the php.ini file, you must manually restore php.ini.');
					}
				} else {
					log_tips(INFO, 'Successfully append openrasp config to ' . $ini_file);
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
	$sub_item = realpath($root_dir) . DIRECTORY_SEPARATOR . $key;
	if (file_exists($sub_item)) {
		if (substr(sprintf('%o', fileperms($sub_item)), -4) != strval($value)) {
			chmod($sub_item, $value);
		}
		if ($key === "plugins") {
			if (array_key_exists("keep-plugin", $options)) {
				major_tips("Skipped update of the official javascript plugin since '--keep-plugin' is set");
			} else {
				major_tips('Updating the official javascript plugin');
				$plugin_source_dir = __DIR__ . DIRECTORY_SEPARATOR . $key;
				if (file_exists($plugin_source_dir)) {
					$official_plugins = scandir($plugin_source_dir);
					foreach ($official_plugins as $pkey => $plugin) {
						if ($plugin === '.'
							|| $plugin === '..'
							|| !is_file($plugin_source_dir . DIRECTORY_SEPARATOR . $plugin)
							|| !endsWith($plugin, '.js')
						) {
							continue;
						}
						update_file_if_need($plugin_source_dir . DIRECTORY_SEPARATOR . $plugin,
							$sub_item . DIRECTORY_SEPARATOR . $plugin, "official plugin");
					}
				}
			}
		} else if ($key === "locale") {
			if (file_exists(__DIR__ . DIRECTORY_SEPARATOR . $key)) {
				clear_dir($sub_item);
				recurse_copy(__DIR__ . DIRECTORY_SEPARATOR . $key, $sub_item);
			}
		} else if ($key === "conf") {
			if (array_key_exists("keep-conf", $options)) {
				major_tips("Skipped update of openrasp config since '--keep-conf' is set");
			} else {
				major_tips('Updating the openrasp config');
				$conf_dir = __DIR__ . DIRECTORY_SEPARATOR . $key;
				if (file_exists($conf_dir)) {
					update_file_if_need($conf_dir . DIRECTORY_SEPARATOR . "openrasp.yml",
						$sub_item . DIRECTORY_SEPARATOR . "openrasp.yml", "openrasp config");
				}
			}
		}
	} else {
		$old_mask = umask(0);
		$mkdir_res = mkdir($sub_item, $value);
		$old_mask = umask(0);
		if (!$mkdir_res) {
			log_tips(ERROR, "Unable to create directory: $sub_item");
		}
		if (file_exists(__DIR__ . DIRECTORY_SEPARATOR . $key)) {
			recurse_copy(__DIR__ . DIRECTORY_SEPARATOR . $key, $sub_item);
		}
	}
	if ($key === "plugins") {
		if (array_key_exists("without-plugin", $options)) {
			clear_dir($sub_item);
			major_tips("All javascript plugins will be removed since '--without-plugin' is set");
		} else if ($remote_enable === "1") {
			clear_dir($sub_item);
			major_tips('All javascript plugins will be removed since remote management is turned on');
		}
	}
}

major_tips("Installation completed without errors, please restart PHP server to take effect.", TRUE); 
?>
