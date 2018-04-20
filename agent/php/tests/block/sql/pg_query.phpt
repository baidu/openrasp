--TEST--
Check for sql connection (pg_query)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("pgsql")) print "skip";
if (!function_exists("pg_query")) print "skip";
require_once('skipifpgconnectfailure.inc');
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
$dbconn = pg_connect("host=".$pg_host." port=".$pg_port." dbname=".$pg_db." user=".$pg_user." password=".$pg_passwd);
if($dbconn) {
  $result=pg_query($dbconn, "SELECT name, id FROM " . $pg_table . " WHERE id=0 and 1=2 union select user, 1#");
  if ($result) {
    echo "pg_query OK";
  }
  pg_close($dbconn);
} else {
  echo "pg_connect error, please make sure the pg server is connectable.";
}
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>