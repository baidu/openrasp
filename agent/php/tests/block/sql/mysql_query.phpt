--TEST--
Check for sql query hook (mysql_query)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("mysql")) print "skip";
if (!function_exists("mysql_query")) print "skip";
require_once('skipifmysqlconnectfailure.inc');
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
@$con = mysql_connect($mysql_host, $mysql_user, $mysql_passwd);
if (!$con)
  {
  die('Could not connect: ' . mysql_error());
  }
mysql_select_db($mysql_db);

$result = mysql_query("SELECT id, name FROM " . $mysql_table . " WHERE id=0 and 1=2 union select user(), 1#");
$num_rows = mysql_num_rows($result);
printf ("ROWS: %d",$num_rows);
mysql_free_result($result);
mysql_close($con);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>