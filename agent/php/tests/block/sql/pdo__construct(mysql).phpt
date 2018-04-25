--TEST--
Check for mysql connection (PDO::__construct)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("pdo")) print "skip";
if (!extension_loaded("pdo_mysql")) print "skip";
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
$dsn = 'mysql:dbname='.$mysql_db.';host='.$mysql_host.';port='.$mysql_port;
try {
    $dbh = new PDO($dsn, 'root', $mysql_passwd);
} catch (PDOException $e) {
    echo 'Connection failed: ' . $e->getMessage();
}
echo "pdo mysql connection OK"
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>