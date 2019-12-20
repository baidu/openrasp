--TEST--
check policy log
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
expose_php=false
display_errors=false
--FILE--
<?php
include(__DIR__.'/timezone.inc');
passthru('tail -n 1 /tmp/openrasp/logs/alarm/alarm.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*sensitive.*