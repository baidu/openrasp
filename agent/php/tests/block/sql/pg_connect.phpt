--TEST--
Check for sql connection (pg_connect)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("pgsql")) print "skip";
if (!function_exists("pg_connect")) print "skip";
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
$dbconn = pg_connect("host=".$pg_host." port=".$pg_port." dbname=".$pg_db." user=postgres password=".$pg_passwd);
if ($dbconn) {
  echo "pg_connect OK";
}
?>
--EXPECT--