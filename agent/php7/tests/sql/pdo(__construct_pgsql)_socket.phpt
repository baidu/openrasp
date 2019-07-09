--TEST--
hook PDO::__construct via unix domain socket
--SKIPIF--
<?php 
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
if (!extension_loaded("pdo")) die("Skipped: pdo extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
new PDO('pgsql:host=/var/run/postgresql/;port=5432;user=postgres;password=postgres');
passthru('tail -n 1 /tmp/openrasp/logs/policy/policy.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*using the high privileged account.*