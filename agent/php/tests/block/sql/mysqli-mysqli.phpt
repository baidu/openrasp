--TEST--
Check for sql connection (mysqli::mysqli)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("mysqli")) print "skip";
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
$mysqli = new mysqli($mysql_host, "root", $mysql_passwd, $mysql_db);

/* check connection */
if ($mysqli->connect_errno) {
    printf("Connect failed: %s\n", $mysqli->connect_error);
    exit();
} else {
  echo "mysqli::mysqli OK";
}

$mysqli->close();
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>