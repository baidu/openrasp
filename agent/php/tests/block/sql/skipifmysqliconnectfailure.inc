<?php
include_once('openrasp_test_sql_config.inc');
$link = mysqli_connect($mysql_host, $mysql_user, $mysql_passwd, $mysql_db);
if (mysqli_connect_errno()) {
    die(sprintf("skip can't connect to MySQL server - %s", mysqli_connect_error()));
}
mysqli_close($link);
?>