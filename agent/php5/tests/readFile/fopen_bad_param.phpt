--TEST--
hook fopen bad param
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(fopen(null,null));
?>
--EXPECTREGEX--
Warning: fopen\(\): Filename cannot be empty.*
