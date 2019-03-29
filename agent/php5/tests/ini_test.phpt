--TEST--
ini test
--SKIPIF--
<?php
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.remote_management_enable=on
openrasp.heartbeat_interval=10000
--FILE--
<?php
include(__DIR__.'/timezone.inc');
echo "no check";
?>
--EXPECTREGEX--
no check