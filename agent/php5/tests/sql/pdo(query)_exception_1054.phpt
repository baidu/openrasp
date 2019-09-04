--TEST--
hook PDO::query exception
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
     sql_exception: {
        action: 'block'
    }
}
EOF;
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysqli")) die("Skipped: mysqli extension required.");
if (!extension_loaded("pdo")) die("Skipped: pdo extension required.");
@$con = mysqli_connect('127.0.0.1', 'root', 'rasp#2019');
if (mysqli_connect_errno()) die("Skipped: can not connect to MySQL " . mysqli_connect_error());
mysqli_close($con);
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
try {
  include('pdo_mysql.inc');
  $con->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
  $con->query("select unknownunknownunknown from mysql.user");
} catch (PDOException $e) {
    echo 'error message: ' . $e->getMessage();
}
?>
--EXPECTREGEX--
error message: SQLSTATE\[42S22\]: Column not found: 1054 Unknown column 'unknownunknownunknown' in 'field list'