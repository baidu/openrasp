--TEST--
hook mysql_connect
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mysql extension in PHP7.');
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysql")) die("Skipped: mysql extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.enforce_policy=On
--FILE--
<?php
mysql_connect('127.0.0.1', 'root');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>