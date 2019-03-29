--TEST--
hook pcntl_exec bad param
--SKIPIF--
<?php
if (!function_exists("pcntl_exec")) die("Skipped: pcntl is disabled.");
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
pcntl_exec();
?>
--EXPECTREGEX--
Warning: pcntl_exec\(\) expects .*