--TEST--
hook pg_pconnect
--SKIPIF--
<?php
$conf = <<<CONF
security.enforce_policy: true
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
pg_pconnect('host=127.0.0.1 port=5432 user=postgres password=postgres');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>