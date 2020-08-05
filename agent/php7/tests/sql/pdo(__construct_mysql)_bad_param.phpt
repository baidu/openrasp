--TEST--
hook PDO::__construct bad param
--SKIPIF--
<?php 
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysqli")) die("Skipped: mysqli extension required.");
if (!extension_loaded("pdo")) die("Skipped: pdo extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
new PDO();
?>
--EXPECTREGEX--
.*PDO::__construct\(\) expects.*