--TEST--
policy log test
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
include(__DIR__.'/skipif.inc');
if (!extension_loaded("mysqli")) die("Skipped: mysqli extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.remote_management_enable=1
openrasp.backend_url=http://127.0.0.1:8383
openrasp.app_id=ea74547f9fa31791425b17a594483630d75ab780
openrasp.app_secret=Fu1O0iXRg3hEq2Im3PiKFsi48SgxUAQ90xp0mitCCqF 
openrasp.heartbeat_interval=10
--GET--
a=force_to_cgi
--CGI--
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
sleep(2);
@mysqli_connect('127.0.0.1', 'root', 'rasp#2019');
@mysqli_connect('127.0.0.1', 'root', 'rasp#2019');
@mysqli_connect('127.0.0.1', 'root', 'rasp#2019');
passthru('tail -n 1 /tmp/openrasp/logs/policy/policy.log.'.date("Y-m-d"));
sleep(18);
?>
--CLEAN--
<?php
include(__DIR__.'/clean.inc');
?>
--EXPECTREGEX--
.*using the high privileged account.*
