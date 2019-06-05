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
@$con = mysql_connect('localhost', get_current_user());
if (!$con) die("Skipped: can not connect to MySQL " . mysql_error() . " by user: " . get_current_user());
mysql_close($con);
?>
--INI--
sql.safe_mode = On
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@mysql_connect();
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>