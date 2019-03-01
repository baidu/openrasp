--TEST--
hook file bad param
--DESCRIPTION--
http://php.net/manual/en/function.file.php
local file
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(file());
?>
--EXPECTREGEX--
Warning: file\(\) expects at least 1 parameter, 0 given in.*