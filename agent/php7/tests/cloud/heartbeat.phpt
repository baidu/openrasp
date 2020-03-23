--TEST--
crash test
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
--ENV--
return <<<END
DOCUMENT_ROOT=/tmp/openrasp
END;
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
sleep(2);
@mysqli_connect('127.0.0.1', 'root', 'rasp#2019');
?>
--CLEAN--
<?php
include(__DIR__.'/clean.inc');
?>
--EXPECT--
