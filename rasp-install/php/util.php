<?php
# whatever to make PHP happy
date_default_timezone_set('Asia/Shanghai');

const UNKNOWN 		= "unknown";

const OS_WIN 		= "win32";
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

function startsWith($haystack, $needle)
{
     $length = strlen($needle);
     return (substr($haystack, 0, $length) === $needle);
}

function endsWith($haystack, $needle)
{
    $length = strlen($needle);

    return $length === 0 || 
    (substr($haystack, -$length) === $needle);
}

function zip_data($source, $destination) {
    if (extension_loaded('zip') === true) {
        if (file_exists($source) === true) {
            $zip = new ZipArchive();
            if ($zip->open($destination, ZIPARCHIVE::CREATE) === true) {
                $source = realpath($source);
                if (is_dir($source) === true) {
                    $files = new RecursiveIteratorIterator(new RecursiveDirectoryIterator($source), RecursiveIteratorIterator::SELF_FIRST);
                    foreach ($files as $file) {
                        $file = realpath($file);
                        if (is_dir($file) === true) {
                            $zip->addEmptyDir(str_replace($source . '/', '', $file . '/'));
                        } else if (is_file($file) === true) {
                            $zip->addFromString(str_replace($source . '/', '', $file), file_get_contents($file));
                        }
                    }
                } else if (is_file($source) === true) {
                    $zip->addFromString(basename($source), file_get_contents($source));
                }
            }
            return $zip->close();
        }
    }
    return false;
}

function clear_dir($dir) {
	$it = new RecursiveDirectoryIterator($dir, RecursiveDirectoryIterator::SKIP_DOTS);
	$files = new RecursiveIteratorIterator($it,
				 RecursiveIteratorIterator::CHILD_FIRST);
	foreach($files as $file) {
		if ($file->isDir()){
			rmdir($file->getRealPath());
		} else {
			unlink($file->getRealPath());
		}
	}
}

function recurse_copy($src,$dst) { 
	$dir = opendir($src); 
	@mkdir($dst); 
    while(false !== ( $file = readdir($dir)) ) { 
        if (( $file != '.' ) && ( $file != '..' )) { 
            if ( is_dir($src . '/' . $file) ) { 
                recurse_copy($src . '/' . $file,$dst . '/' . $file); 
            } 
            else { 
                copy($src . '/' . $file,$dst . '/' . $file); 
            } 
        } 
    } 
    closedir($dir); 
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
function show_help($help_msg) {
	echo $help_msg;
	exit(0); 
}

//通用全局变量
$index 				= 1;
$root_dir 			= null;
$current_os 		= get_OS();
$supported_sapi 	= array('apache2', 'cli', 'fpm');
$lib_filename 		= $current_os == OS_WIN ? 'php_openrasp.dll' : 'openrasp.so';
$extension_dir 		= ini_get('extension_dir');
$ini_loaded_file 	= php_ini_loaded_file();
$ini_scanned_path 	= get_ini_scanned_path();
//make sure loaded after json and pdo
$ini_scanned_file 	= 'z_openrasp.ini';
$openrasp_work_sub_folders = array('conf'=>0755, 'assets'=>0755, 'logs'=>0777, 'locale'=>0755, 'plugins'=>0755);

?>