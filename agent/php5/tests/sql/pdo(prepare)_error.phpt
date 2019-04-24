--TEST--
hook PDO::prepare
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('sql', params => {
    assert(params.query == 'SELECT a FROM b WHERE c < :c')
    assert(params.server == 'mysql')
    return block
})
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
include('pdo_mysql.inc');
$con->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
$con->prepare('SELECT a FROM b WHERE c < :c', array(PDO::ATTR_CURSOR => PDO::CURSOR_ERROR));
?>
--EXPECTREGEX--
Fatal error: (Uncaught Error: Undefined class constant|Undefined class constant 'CURSOR_ERROR').*