--TEST--
hook mysqli_connect
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysqli")) die("Skipped: mysqli extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.enforce_policy=On
--FILE--
<?php
mysqli_connect('127.0.0.1', 'root');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>