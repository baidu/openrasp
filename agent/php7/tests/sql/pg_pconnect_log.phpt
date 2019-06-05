--TEST--
hook pg_pconnect security.enforce_policy: false
--SKIPIF--
<?php
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("pgsql")) die("Skipped: pgsql extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include('pg_pconnect.inc');
?>
--EXPECTREGEX--
