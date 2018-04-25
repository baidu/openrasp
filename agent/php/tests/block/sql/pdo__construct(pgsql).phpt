--TEST--
Check for pgsql connection (PDO::__construct)
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("pdo")) print "skip";
if (!extension_loaded("pdo_pgsql")) print "skip";
?>
--FILE--
<?php
include('openrasp_test_sql_config.inc');
$dsn = 'pgsql:dbname='.$pg_db.';host='.$pg_host.';port='.$pg_port.';user=postgres;password='.$pg_passwd;
try {
    $dbh = new PDO($dsn);
} catch (PDOException $e) {
    echo 'Connection failed: ' . $e->getMessage();
}
echo "pdo pgsql connection OK"
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>