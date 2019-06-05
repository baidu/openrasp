--TEST--
hook copy bad param
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
copy();
?>
--EXPECTREGEX--
Warning: copy\(\) expects .*