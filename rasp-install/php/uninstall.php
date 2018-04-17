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
OpenRASP Installer for PHP servers - Copyright ©2017-2018 Baidu Inc.
For more details visit: https://rasp.baidu.com/doc/install/software.html

<?php
include_once(__DIR__ . '/util.php');

//全局变量
$root_dir 		    = ini_get('openrasp.root_dir');
$index 				= 1;
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
$openrasp_work_sub_folders = array('conf'=>0755, 'assets'=>0755, 'logs'=>0777, 'locale'=>0755, 'plugins'=>0755);
$help_msg = <<<HELP
Synopsis:
    php uninstall.php [options]

Options:

    -h          		This Help.

HELP;

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> 过程化卸载 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
$shortopts = "h";
$options = getopt($shortopts);
if (array_key_exists("h", $options)) {
	show_help();
}

if(!extension_loaded('openrasp')) {
    log_tips(ERROR, 'OpenRASP PHP extension is not loaded.');
}

major_tips('Processing INI files');
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

    if (!empty($ini_system_links) && is_array($ini_system_links)) {
        foreach ($ini_system_links as $key => $value) {
            if (file_exists($value) && is_link($value) && readlink($value) === $ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file) {
                log_tips(INFO, 'Remove found system links of openrasp.ini: '.$value);
                unlink($value);
            }
        }
    }

    unlink($ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file);
    log_tips(INFO, 'Remove: '.$ini_scanned_path.DIRECTORY_SEPARATOR.$ini_scanned_file);
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
        log_tips(ERROR, "Unable to locate OPENRASP closing tags in '$ini_loaded_file', possibly corrupted ini file");
    } else if (FINSH === $found_openrasp) {
        log_tips(INFO, "Located OpenRASP configuration in INI files, removing");
    }
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
             log_tips(INFO, 'Fail to remove openrasp config from '.$ini_loaded_file.', you must manually remove them.');
             if (!copy($ini_loaded_file.'.bak', $ini_loaded_file)) {
                log_tips(ERROR, 'Fail to restore the original php.ini file.');
            }
         } else {
             log_tips(INFO, 'Successfully remove openrasp config from '.$ini_loaded_file);
         }
        fclose($handle);
    } else {
        log_tips(ERROR, 'Cannot open '.$ini_loaded_file);
    } 
    unlink($ini_loaded_file.'.bak');
} else {
    log_tips(ERROR, 'Cannot find appropriate php.ini file.');
}

major_tips('Removing OpenRASP PHP extensions');
if (!file_exists($extension_dir)) {
	log_tips(ERROR, $extension_dir.' not exist!');
}
if (!is_writable($extension_dir)) {
	log_tips(ERROR, $extension_dir.' is not writable, please make sure you have write permissions!');
}
$lib_dest_path = $extension_dir.DIRECTORY_SEPARATOR.$lib_filename;
if (file_exists($lib_dest_path)) {
    if (unlink($lib_dest_path)) {
        log_tips(INFO, "'$lib_dest_path' removed");
    } else {
        log_tips(ERROR, 'Fail to delete '.$lib_dest_path);
    }
}
$lib_backup_path = $lib_dest_path.'.bak';
if (file_exists($lib_backup_path)) {
    if (unlink($lib_backup_path)) {
        log_tips(INFO, 'Successfully delete backup file '.$lib_backup_path);
    } else {
        log_tips(ERROR, 'Fail to delete backup file '.$lib_backup_path);
    }
}

major_tips('Removing OpenRASP work folder');
if (!file_exists($root_dir) && !is_writable($root_dir)) {
	log_tips(ERROR, $root_dir.' is not writable, please make sure you have write permissions!');
}
foreach($openrasp_work_sub_folders as $key => $value) {
    $sub_item = realpath($root_dir).DIRECTORY_SEPARATOR.$key;
    clear_dir($sub_item);
    rmdir($sub_item);
}
if (rmdir($root_dir)) {
    log_tips(INFO, "'$root_dir' removed");
} else {
    log_tips(INFO, 'non-openrasp file found in '.$root_dir.', fail to remove it.');
}


major_tips('Uninstallation completed without errors, please restart or reload PHP server to take effect.', TRUE); 
?>
