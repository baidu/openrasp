--TEST--
hook mysql_pconnect bad param
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mysql extension in PHP7.');
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysql")) die("Skipped: mysql extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
mysql_pconnect(array());
?>
--EXPECTREGEX--
.*Warning: mysql_pconnect\(\) expects.*