--TEST--
Check for sql connection (mysql_connect)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("mysql")) print "skip";
if (!function_exists("mysql_connect")) print "skip";
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
@$con = mysql_connect($mysql_host, 'root', $mysql_passwd);
if (!$con) {
  die('Could not connect: ' . mysql_error());
} else {
  echo "mysql_connect OK";
}
mysql_close($con);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>