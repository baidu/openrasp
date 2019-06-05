--TEST--
hook file_put_contents bad param
--SKIPIF--
<?php
$dir = __DIR__;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'test');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
file_put_contents();
?>
--EXPECTREGEX--
Warning: file_put_contents\(\) expects at least 2 parameters, 0 given in.*