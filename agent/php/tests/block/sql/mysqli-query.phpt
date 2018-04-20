--TEST--
Check for sql query hook (mysqli::query)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("mysqli")) print "skip";
require_once('skipifmysqliconnectfailure.inc');
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
$mysqli = new mysqli($mysql_host, $mysql_user, $mysql_passwd, $mysql_db);

/* check connection */
if ($mysqli->connect_errno) {
    printf("Connect failed: %s\n", $mysqli->connect_error);
    exit();
}

/* Select queries return a resultset */
if ($result = $mysqli->query("SELECT id, name FROM " . $mysql_table . " WHERE id=0 and 1=2 union select user(), 1#")) {
    printf("Select returned %d rows.\n", $result->num_rows);

    /* free result set */
    $result->close();
}

$mysqli->close();
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>