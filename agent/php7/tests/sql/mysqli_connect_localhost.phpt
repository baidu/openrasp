--TEST--
hook mysqli_connect via unix domain socket
--SKIPIF--
<?php
$conf = <<<CONF
security.enforce_policy: true
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysqli")) die("Skipped: mysqli extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
mysqli_connect('localhost', 'root', 'rasp#2019');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>