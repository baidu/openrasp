--TEST--
hook mysql_connect safe mode
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mysql extension in PHP7.');
$conf = <<<CONF
security.enforce_policy: true
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysql")) die("Skipped: mysql extension required.");
?>
--INI--
sql.safe_mode = On
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@mysql_connect();
?>
--EXPECTREGEX--
