--TEST--
ini invalid rasp_id
--SKIPIF--
<?php
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.remote_management_enable=0
openrasp.rasp_id=123456
openrasp.heartbeat_interval=10000
--GET--
a=force_to_cgi
--CGI--
--FILE--
<?php
include(__DIR__.'/timezone.inc');
echo "no check";
?>
--EXPECTREGEX--
.*openrasp.rasp_id can only contain alphanumeric characters and is between 16 and 512 in length.*