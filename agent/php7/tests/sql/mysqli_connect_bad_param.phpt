--TEST--
hook mysqli_connect bad param
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysqli")) die("Skipped: mysqli extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
mysqli_connect(array());
?>
--EXPECTREGEX--
Warning: mysqli_connect\(\) expects.*