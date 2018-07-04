--TEST--
hook PDO::__construct
--SKIPIF--
<?php 
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
if (!extension_loaded("pdo")) die("Skipped: pdo extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.enforce_policy=On
--FILE--
<?php
new PDO('pgsql:host=127.0.0.1;port=5432;user=postgres');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>