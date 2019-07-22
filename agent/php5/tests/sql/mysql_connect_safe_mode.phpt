--TEST--
hook mysql_connect safe mode
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mysql extension in PHP7.');
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysql")) die("Skipped: mysql extension required.");
$current_user = get_current_user();
if ($current_user != "root") die("Skipped: current user is not root");
@$con = mysql_connect('localhost', $current_user);
if (!$con) die("Skipped: can not connect to MySQL " . mysql_error() . " by user: " . $current_user);
mysql_close($con);
?>
--INI--
sql.safe_mode = On
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
@mysql_connect();
passthru('tail -n 1 /tmp/openrasp/logs/policy/policy.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*using the high privileged account.*
