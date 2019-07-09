--TEST--
hook pg_pconnect PGSQL_CONNECT_FORCE_NEW
--SKIPIF--
<?php
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
pg_pconnect('host=127.0.0.1 port=5432 user=postgres password=postgres', PGSQL_CONNECT_FORCE_NEW);
passthru('tail -n 1 /tmp/openrasp/logs/policy/policy.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*using the high privileged account.*