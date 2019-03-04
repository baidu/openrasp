--TEST--
hook SQLite3::exec bad param
--SKIPIF--
<?php
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("sqlite3")) die("Skipped: sqlite3 extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$db = new SQLite3('test.db');
$results = $db->exec(array());
$db->close();
?>
--EXPECTREGEX--
Warning: SQLite3::exec\(\) expects.*