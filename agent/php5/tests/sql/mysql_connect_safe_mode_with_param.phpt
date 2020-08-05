--TEST--
hook mysql_connect safe mode with param
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mysql extension in PHP7.');
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysql")) die("Skipped: mysql extension required.");
?>
--INI--
sql.safe_mode = On
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
mysql_connect('127.0.0.1', 'root', 'rasp#2019');
?>
--EXPECTREGEX--
.*Notice: mysql_connect\(\): SQL safe mode in effect.*