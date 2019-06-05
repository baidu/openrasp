--TEST--
hook readfile bad param
--SKIPIF--
<?php
$plugin = <<<EOF
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(readfile());
?>
--EXPECTREGEX--
Warning: readfile\(\) expects at least 1 parameter, 0 given in.*