--TEST--
hook exec bad param (with stack)
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
exec();
?>
--EXPECTREGEX--
Warning: exec\(\) expects .*