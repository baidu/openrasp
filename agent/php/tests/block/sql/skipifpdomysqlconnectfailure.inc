<?php
include_once('openrasp_test_sql_config.inc');
$dsn = 'mysql:dbname='.$mysql_db.';host='.$mysql_host.';port='.$mysql_port;
try {
    $dbh = new PDO($dsn, $mysql_user, $mysql_passwd);
    $dbh = null;
} catch (PDOException $e) {
    die(sprintf("skip can't connect to MySQL server -" . $e->getMessage()));
}
?>