--TEST--
hook PDO::__construct remote ip
--SKIPIF--
<?php 
$conf = <<<CONF
security.enforce_policy=true
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysqli")) die("Skipped: mysqli extension required.");
if (!extension_loaded("pdo")) die("Skipped: pdo extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
new PDO('mysql:host=192.168.1.100;port=3306', 'root');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>