--TEST--
ini empty app_secret
--SKIPIF--
<?php
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.remote_management_enable=on
openrasp.backend_url=http://openrasp.com
openrasp.app_id=ea74547f9fa31791425b17a594483630d75ab780
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
.*openrasp.app_secret is required.*