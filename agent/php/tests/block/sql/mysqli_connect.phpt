--TEST--
Check for sql connection (mysqli_connect)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("mysqli")) print "skip";
if (!function_exists("mysqli_connect")) print "skip";
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
$link = mysqli_connect($mysql_host, "root", $mysql_passwd, $mysql_db);

/* check connection */
if (mysqli_connect_errno()) {
    printf("Connect failed: %s\n", mysqli_connect_error());
    exit();
} else {
  echo "mysqli_connect OK";
}

mysqli_close($link);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>