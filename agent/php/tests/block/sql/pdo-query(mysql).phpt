--TEST--
Check for mysql connection (PDO::__construct)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("pdo")) print "skip";
if (!extension_loaded("pdo_mysql")) print "skip";
require_once('skipifpdomysqlconnectfailure.inc');
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
$dsn = 'mysql:dbname='.$mysql_db.';host='.$mysql_host.';port='.$mysql_port;
try {
    $dbh = new PDO($dsn, $mysql_user, $mysql_passwd);
    $res = $dbh->query("SELECT * FROM students LIMIT 10");
    echo 'ROW COUNT:'.$res->rowCount();
    $dbh = null;
} catch (PDOException $e) {
    echo 'Query failed: ' . $e->getMessage();
}
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>