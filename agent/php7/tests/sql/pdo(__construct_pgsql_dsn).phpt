--TEST--
hook PDO::__construct
--SKIPIF--
<?php 
$conf = <<<CONF
security.enforce_policy: true
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
if (!extension_loaded("pdo")) die("Skipped: pdo extension required.");
file_put_contents('/tmp/openrasp/pqsql_connect', 'pgsql:host=127.0.0.1;port=5432;user=postgres;password=postgres');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
new PDO('uri:file:///tmp/openrasp/pqsql_connect');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>