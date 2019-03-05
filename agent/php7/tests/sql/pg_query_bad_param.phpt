--TEST--
hook pg_query bad param
--SKIPIF--
<?php
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
include('pg_connect.inc');
if (!$con) die("Skipped: can not connect to postgresql");
pg_close($con);
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include('pg_connect.inc');
pg_query(null, null);
pg_close($con);
?>
--EXPECTREGEX--
Warning: pg_query\(\) expects.*