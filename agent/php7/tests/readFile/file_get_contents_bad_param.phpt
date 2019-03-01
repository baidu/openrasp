--TEST--
hook file_get_contents bad param
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(file_get_contents());
?>
--EXPECTREGEX--
Warning: file_get_contents\(\) expects at least 1 parameter, 0 given in.*