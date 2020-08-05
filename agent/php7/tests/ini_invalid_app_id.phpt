--TEST--
ini invalid app_id
--SKIPIF--
<?php
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.remote_management_enable=on
openrasp.backend_url=http://openrasp.com
openrasp.app_id=123456
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
.*openrasp.app_id must be exactly 40 characters.*