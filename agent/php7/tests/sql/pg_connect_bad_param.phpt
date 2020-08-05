--TEST--
hook pg_connect bad param
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
pg_connect();
?>
--EXPECTREGEX--
Warning: Wrong parameter count.*