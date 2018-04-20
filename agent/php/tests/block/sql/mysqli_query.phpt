--TEST--
Check for sql query hook (mysqli_query)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("mysqli")) print "skip";
if (!function_exists("mysqli_query")) print "skip";
require_once('skipifmysqliconnectfailure.inc');
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
$link = mysqli_connect($mysql_host, $mysql_user, $mysql_passwd, $mysql_db);

/* check connection */
if (mysqli_connect_errno()) {
    printf("Connect failed: %s\n", mysqli_connect_error());
    exit();
}

/* Select queries return a resultset */
if ($result = mysqli_query($link, "SELECT id, name FROM " . $mysql_table . " WHERE id=0 and 1=2 union select user(), 1#")) {
    printf("Select returned %d rows.\n", mysqli_num_rows($result));
    /* free result set */
    mysqli_free_result($result);
}

mysqli_close($link);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>