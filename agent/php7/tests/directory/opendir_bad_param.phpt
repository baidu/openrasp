--TEST--
hook opendir bad param (relative path)
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(opendir());
?>
--EXPECTREGEX--
Warning: opendir\(\) expects .*