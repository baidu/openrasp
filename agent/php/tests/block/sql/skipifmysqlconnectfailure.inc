<?php
include_once('openrasp_test_sql_config.inc');
@$con = mysql_connect($mysql_host, $mysql_user, $mysql_passwd);
if (!$con)
{
    die(sprintf("skip can't connect to MySQL server -" . mysql_error()));
}
mysql_close($con);
?>