<?php
/*
 * Copyright 2017-2020 Baidu Inc.
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
OpenRASP Installer for PHP servers - Copyright 2017-2020 Baidu Inc.
For more details visit: https://rasp.baidu.com/doc/install/software.html

<?php
error_reporting(E_ALL);

foreach (array('/sys/fs/selinux/enforce', '/selinux/enforce') as $selinux) {
	if (!file_exists($selinux)) {
		continue;
	}

	if (@file_get_contents($selinux) == "1") {
		echo "ERROR: selinux is enabled, try disable it with 'setenforce 0'\n";
		exit;
	}
}

if (PHP_VERSION_ID < 50300) {
	echo sprintf("Error: OpenRASP works on PHP 5.3 and onwards, version %s.%s is not supported\n", PHP_MAJOR_VERSION, PHP_MINOR_VERSION);
	exit;
}
include_once(__DIR__ . DIRECTORY_SEPARATOR . 'util.php');

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
	$lib_abspath = sprintf(
		"%s%sphp%s%s%s-php%s.%s-%s%s%s",
		__DIR__,
		DIRECTORY_SEPARATOR,
		$zts_suffix,
		DIRECTORY_SEPARATOR,
		$current_os,
		PHP_MAJOR_VERSION,
		PHP_MINOR_VERSION,
		$machine_type,
		DIRECTORY_SEPARATOR,
		$lib_filename
	);
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

function startWith($str, $needle)
{
	return strpos($str, $needle) === 0;
}

function validateRootDir($root_dir)
{
	// 未配置直接返回
	$open_basedir = ini_get('open_basedir');
	if (empty($open_basedir)) {
		return;
	}

	// 避免因为斜杠匹配失败，暂时不做 realpath 处理
	$root_dir = rtrim($root_dir, '/');
	$valid    = false;

	// TODO: 改为真实路径 + 前缀检查
	$allowed = explode(':', $open_basedir);
	foreach ($allowed as $dir)
	{
		$dir = rtrim($dir, '/');
		if ($dir == $root_dir)
		{
			$valid = true;
			break;
		}
	}

	if (! $valid)
	{
		echo "WARNING: open_basedir is configured and might affect the installation process\n";
		echo "Consider adding $root_dir to your open_basedir config\n";
		echo "Current value: $open_basedir\n";
		echo "\n";
	}
}

class IniConfig
{
	var $extension;
	var $root_dir;
	var $backend_url;
	var $app_id;
	var $app_secret;
	var $rasp_id;
	var $remote_management_enable;
	var $iast_enable;
	var $heartbeat_interval;

	function __construct($extension)
	{
		$this->extension = $extension;
		$this->remote_management_enable = "0";
		$this->iast_enable = "0";
		$this->heartbeat_interval = 180;
	}

	public function setRootDir($root_dir)
	{
		$real_root_dir = realpath($root_dir);
		if (empty($real_root_dir)) {
			log_tips(ERROR, "Can't resolve realpath of " . $root_dir . ": No such directory.");
		} else {
			log_tips(INFO, "openrasp.root_dir => " . $real_root_dir);
			$this->root_dir = $real_root_dir;
		}
	}

	public function getRootDir()
	{
		return $this->root_dir;
	}

	public function setBackendUrl($backend_url)
	{
		if (!empty($backend_url)) {
			if (parse_url($backend_url)) {
				log_tips(INFO, "openrasp.backend_url => " . $backend_url);
				$this->backend_url = $backend_url;
			} else {
				log_tips(ERROR, "backend-url option is an illegal URL.");
			}
		} else {
			log_tips(ERROR, "backend-url option cannot be empty.");
		}
	}

	public function setAppId($app_id)
	{
		if (!empty($app_id)) {
			if (preg_match("/^[0-9a-fA-F]{40}$/", $app_id) != 0) {
				log_tips(INFO, "openrasp.app_id => " . $app_id);
				$this->app_id = $app_id;
			} else {
				log_tips(ERROR, "app-id option format is incorrect.");
			}
		} else {
			log_tips(ERROR, "app-id option cannot be empty.");
		}
	}

	public function setAppSecret($app_secret)
	{
		if (!empty($app_secret)) {
			if (preg_match("/^[0-9a-zA-Z_-]{43,45}$/", $app_secret) != 0) {
				log_tips(INFO, "openrasp.app_secret => " . $app_secret);
				$this->app_secret = $app_secret;
			} else {
				log_tips(ERROR, "app-secret option format is incorrect.");
			}
		} else {
			log_tips(ERROR, "app-secret option cannot be empty.");
		}
	}

	public function setRaspId($rasp_id)
	{
		if (!empty($rasp_id)) {
			if (preg_match("/^[0-9a-zA-Z]{16,512}$/", $rasp_id) != 0) {
				log_tips(INFO, "openrasp.rasp_id => " . $rasp_id);
				$this->rasp_id = $rasp_id;
			} else {
				log_tips(ERROR, "rasp-id option format is incorrect.");
			}
		} else {
			log_tips(ERROR, "rasp-id option cannot be empty.");
		}
	}

	public function setHeartbeatInterval($heartbeat_interval)
	{
		if (is_numeric($heartbeat_interval) && (int) $heartbeat_interval >= 10 && (int) $heartbeat_interval <= 1800) {
			log_tips(INFO, "openrasp.heartbeat_interval => " . $heartbeat_interval);
			$this->heartbeat_interval = $heartbeat_interval;
		} else {
			log_tips(ERROR, "heartbeat option must be numeric and between 10 and 1800.");
		}
	}

	public function isRemoteManagementEnable()
	{
		return $this->remote_management_enable === "1";
	}

	public function generateRemoteManagementEnable()
	{
		if (!empty($this->backend_url) && !empty($this->app_id) && !empty($this->app_secret)) {
			$this->remote_management_enable = "1";
		} else if (!empty($this->backend_url) || !empty($this->app_id) || !empty($this->app_secret)) {
			log_tips(ERROR, "backend-url app-id app-secret options must be specified simultaneously.");
		}
	}

	public function enableIast()
	{
		$this->iast_enable = "1";
	}

	public function isIastEnable()
	{
		return $this->iast_enable === "1";
	}

	public function initializeRootDir()
	{
		if (file_exists($this->root_dir)) {
			if (!chmod($this->root_dir, 0777)) {
				log_tips(ERROR, 'Fail to chmod ' . $this->root_dir);
			}
		} else {
			if (!mkdir($this->root_dir, 0777, TRUE)) {
				log_tips(ERROR, 'Unable to create directory: ' . $this->root_dir);
			}
		}
	}

	public function get_ini_content()
	{
		$ini_content = <<<OPENRASP
;OPENRASP BEGIN
	
extension=$this->extension
openrasp.root_dir=$this->root_dir
	
;国际化配置
;openrasp.locale=

;云端地址
openrasp.backend_url=$this->backend_url

;agent app_id
openrasp.app_id=$this->app_id

;agent secret
openrasp.app_secret=$this->app_secret

;agent rasp_id
openrasp.rasp_id=$this->rasp_id

;远程管理开关
openrasp.remote_management_enable=$this->remote_management_enable

;心跳时间间隔
openrasp.heartbeat_interval=$this->heartbeat_interval

;SSL证书验证开关
openrasp.ssl_verifypeer=0

;IAST开关
openrasp.iast_enable=$this->iast_enable
	
;OPENRASP END

OPENRASP;
		return $ini_content;
	}
}

class RaspOption
{
	const NOVALUE = 0;
	const REQUIRED = 1;
	const OPTIONAL = 2;

	var $shortOpts;
	var $longOpts;

	function __construct()
	{
		$this->shortOpts = array(array(), array(), array());
		$this->longOpts = array(array(), array(), array());
	}

	//only support one by one
	public function addShortOption($key, $type)
	{
		if (is_string($key) && strlen($key) == 1) {
			switch ($type) {
				case self::OPTIONAL:
					array_push($this->shortOpts[self::OPTIONAL], $key);
					break;
				case self::REQUIRED:
					array_push($this->shortOpts[self::REQUIRED], $key);
					break;
				case self::NOVALUE:
				default:
					array_push($this->shortOpts[self::NOVALUE], $key);
					break;
			}
		}
	}

	public function addLongOption($key, $type)
	{
		if (is_string($key)) {
			switch ($type) {
				case self::OPTIONAL:
					array_push($this->longOpts[self::OPTIONAL], $key);
					break;
				case self::REQUIRED:
					array_push($this->longOpts[self::REQUIRED], $key);
					break;
				case self::NOVALUE:
				default:
					array_push($this->longOpts[self::NOVALUE], $key);
					break;
			}
		}
	}

	private function getShortOptionStr()
	{
		$str = '';

		foreach ($this->shortOpts[self::NOVALUE] as $opt) {
			$str .= $opt;
		}
		foreach ($this->shortOpts[self::REQUIRED] as $opt) {
			$str .= ($opt . ':');
		}
		foreach ($this->shortOpts[self::OPTIONAL] as $opt) {
			$str .= ($opt . '::');
		}
		return $str;
	}

	private function getLongOptionArr()
	{
		$arr = array();

		foreach ($this->longOpts[self::NOVALUE] as $opt) {
			array_push($arr, $opt);
		}
		foreach ($this->longOpts[self::REQUIRED] as $opt) {
			array_push($arr, ($opt . ':'));
		}
		foreach ($this->longOpts[self::OPTIONAL] as $opt) {
			array_push($arr, ($opt . '::'));
		}
		return $arr;
	}


	public function checkOptions()
	{
		global $argc;
		global $argv;
		$size = sizeof($argv);
		for ($i = 1; $i < $size; $i++) {
			//long option
			if (startWith($argv[$i], "--")) {
				$opt = substr($argv[$i], 2);
				if (in_array($opt, $this->longOpts[self::NOVALUE])) {
					continue;
				}
				foreach ($this->longOpts[self::OPTIONAL] as $optionalOpt) {
					if (startWith($opt, $optionalOpt . '=')) {
						continue 2;
					}
				}
				foreach ($this->longOpts[self::REQUIRED] as $requiredOpt) {
					if ($opt == $requiredOpt) {
						$i++;
						continue 2;
					}
					if (startWith($opt, $requiredOpt . '=')) {
						continue 2;
					}
				}
				return '--' . $opt;
			}
			//short option
			if (startWith($argv[$i], "-")) {
				$opt = substr($argv[$i], 1);
				if (in_array($opt, $this->shortOpts[self::NOVALUE])) {
					continue;
				}
				foreach ($this->shortOpts[self::OPTIONAL] as $optionalOpt) {
					if (startWith($opt, $optionalOpt . '=')) {
						continue 2;
					}
				}
				foreach ($this->shortOpts[self::REQUIRED] as $requiredOpt) {
					if ($opt == $requiredOpt) {
						$i++;
						continue 2;
					}
					if (startWith($opt, $requiredOpt)) {
						continue 2;
					}
				}
				return '-' . $opt;
			}
			return $argv[$i];
		}
		return false;
	}

	public function getOptions()
	{
		return getopt($this->getShortOptionStr(), $this->getLongOptionArr());
	}
}

$iniConfig = new IniConfig($lib_filename);
$raspOption = new RaspOption();
$lib_source_path = get_lib_2b_installed($current_os, $lib_filename);
$install_help_msg = <<<HELP
Synopsis:
    php install.php [options]

Options:
    -d <openrasp_root>      Specify OpenRASP installation folder (required)

    --backend-url <url>     Value of backend_url (required for remote management)

    --app-id <id>           Value of app_id (required for remote management)

    --app-secret <secret>   Value of app_secret (required for remote management)

    --rasp-id <id>          Value of rasp_id (if not set, it will be generated at runtime)

    --heartbeat             Value of heartbeat interval (10 - 1800)

    --keep-ini              Do not update PHP ini entries

    --keep-plugin           Do not update the official javascript plugin (higher priority than --without-plugin)

    --keep-conf             Do not update the openrasp config in root_dir/conf directory

    --without-plugin        Do not install the official javascript plugin

    --iast                  Enable IAST mode

    -h, --help              Show help messages

HELP;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 参数解析 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
$raspOption->addShortOption('d', RaspOption::REQUIRED);
$raspOption->addShortOption('h', RaspOption::NOVALUE);
$raspOption->addLongOption('iast', RaspOption::NOVALUE);
$raspOption->addLongOption('without-plugin', RaspOption::NOVALUE);
$raspOption->addLongOption('keep-ini', RaspOption::NOVALUE);
$raspOption->addLongOption('keep-plugin', RaspOption::NOVALUE);
$raspOption->addLongOption('keep-conf', RaspOption::NOVALUE);
$raspOption->addLongOption('help', RaspOption::NOVALUE);
$raspOption->addLongOption('app-id', RaspOption::REQUIRED);
$raspOption->addLongOption('app-secret', RaspOption::REQUIRED);
$raspOption->addLongOption('rasp-id', RaspOption::REQUIRED);
$raspOption->addLongOption('backend-url', RaspOption::REQUIRED);
$raspOption->addLongOption('heartbeat', RaspOption::REQUIRED);

$invalidOption = $raspOption->checkOptions();
if ($invalidOption) {
	log_tips(ERROR, "invalid option '$invalidOption', try '--help or -h' for more information.");
}
$options = $raspOption->getOptions();

if (array_key_exists("h", $options) || array_key_exists("help", $options)) {
	show_help($install_help_msg);
}
if (array_key_exists("d", $options) && !empty($options["d"])) {
	// 创建目录
	if (is_string($options["d"])) {
		// 检查 open_basedir 配置
		validateRootDir($options["d"]);

		if (!file_exists($options["d"])) {
			mkdir($options["d"], 0777, true);
		}
		$iniConfig->setRootDir($options["d"]);
	} else {
		log_tips(ERROR, "openrasp.root_dir must be string, please make sure all option names are correct");
	}
} else {
	log_tips(ERROR, "openrasp.root_dir must be specified via option \"-d\"");
}

if (array_key_exists("backend-url", $options)) {
	$iniConfig->setBackendUrl($options["backend-url"]);
}

if (array_key_exists("app-id", $options)) {
	$iniConfig->setAppId($options["app-id"]);
}

if (array_key_exists("app-secret", $options)) {
	$iniConfig->setAppSecret($options["app-secret"]);
}

if (array_key_exists("rasp-id", $options)) {
	$iniConfig->setRaspId($options["rasp-id"]);
}

if (array_key_exists("heartbeat", $options)) {
	$iniConfig->setHeartbeatInterval($options["heartbeat"]);
}

if (array_key_exists("iast", $options)) {
	$iniConfig->enableIast();
}

$iniConfig->generateRemoteManagementEnable();

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 检查依赖扩展 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
major_tips('Check whether required PHP extensions are installed');
$dep_exts = array('json', 'PDO');
if (!check_dep_exts_installed($dep_exts)) {
	log_tips(ERROR, "OpenRASP depends on the following PHP extension: " . implode(" ", $dep_exts) . ". Make sure they are installed on your system.");
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 拷贝动态库 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
major_tips('Installing OpenRASP PHP extension');
if (!file_exists($extension_dir)) {
	if (!mkdir($extension_dir, 0777, TRUE)) {
		log_tips(ERROR, 'Unable to create extension directory: ' . $extension_dir);
	}
}
if (!is_writable($extension_dir)) {
	log_tips(ERROR, "Extension directory '$extension_dir' is not writable, make sure you have write permissions");
}
if (!file_exists($lib_source_path)) {
	log_tips(ERROR, "Unsupported system or php: " . phpversion() . "\nUname: " . php_uname() .
		"\nExpecting '$lib_source_path' to be present." .
		"\nPlease check your system, php version and ZTS state.");
}
$lib_dest_path = $extension_dir . DIRECTORY_SEPARATOR . $lib_filename;
if (
	file_exists($lib_dest_path)
	&& !rename($lib_dest_path, $lib_dest_path . '.bak')
) {
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
			if (fwrite($handle, $iniConfig->get_ini_content()) === FALSE) {
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
			$tmp_ini_data[] = $iniConfig->get_ini_content();
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
$iniConfig->initializeRootDir();
foreach ($openrasp_work_sub_folders as $key => $value) {
	$sub_item = $iniConfig->getRootDir() . DIRECTORY_SEPARATOR . $key;
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
						if (
							$plugin === '.'
							|| $plugin === '..'
							|| !is_file($plugin_source_dir . DIRECTORY_SEPARATOR . $plugin)
							|| !endsWith($plugin, '.js')
						) {
							continue;
						}
						update_file_if_need(
							$plugin_source_dir . DIRECTORY_SEPARATOR . $plugin,
							$sub_item . DIRECTORY_SEPARATOR . $plugin,
							"official plugin"
						);
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
				$conf_filename = $iniConfig->isIastEnable() ? "iast.yml" : "openrasp.yml";
				$conf_dir = __DIR__ . DIRECTORY_SEPARATOR . $key;
				if (file_exists($conf_dir)) {
					update_file_if_need(
						$conf_dir . DIRECTORY_SEPARATOR . $conf_filename,
						$sub_item . DIRECTORY_SEPARATOR . "openrasp.yml",
						"openrasp config (" . $conf_filename . ")"
					);
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
			if ($key === "conf") {
				$conf_filename = $iniConfig->isIastEnable() ? "iast.yml" : "openrasp.yml";
				update_file_if_need(
					__DIR__ . DIRECTORY_SEPARATOR . $key . DIRECTORY_SEPARATOR . $conf_filename,
					$sub_item . DIRECTORY_SEPARATOR . "openrasp.yml",
					"openrasp config (" . $conf_filename . ")"
				);
			} else {
				recurse_copy(__DIR__ . DIRECTORY_SEPARATOR . $key, $sub_item);
			}
		}
	}
	if ($key === "plugins") {
		if (array_key_exists("without-plugin", $options)) {
			clear_dir($sub_item);
			major_tips("All javascript plugins will be removed since '--without-plugin' is set");
		} else if ($iniConfig->isRemoteManagementEnable()) {
			clear_dir($sub_item);
			major_tips('All javascript plugins will be removed since remote management is turned on');
		}
	}
}

major_tips("Installation completed without errors, please restart PHP server to take effect.", TRUE);
?>