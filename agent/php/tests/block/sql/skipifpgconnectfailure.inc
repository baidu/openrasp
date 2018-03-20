<?php
include_once('openrasp_test_sql_config.inc');
$dbconn = @pg_connect("host=".$pg_host." port=".$pg_port." dbname=".$pg_db." user=".$pg_user." password=".$pg_passwd);
if(!$dbconn) {
    die(sprintf("skip can't connect to pg server."));
}
?>