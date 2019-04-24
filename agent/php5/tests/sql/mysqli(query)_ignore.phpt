--TEST--
hook mysqli::query
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('sql', params => {
    return ignore
})
EOF;
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysqli")) die("Skipped: mysqli extension required.");
@$con = mysqli_connect('127.0.0.1', 'root', 'rasp#2019');
if (mysqli_connect_errno()) die("Skipped: can not connect to MySQL " . mysqli_connect_error());
mysqli_close($con);
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@$con = new mysqli('127.0.0.1', 'root', 'rasp#2019');
$con->query('SELECT a FROM b');
$con->close();
?>
--EXPECTREGEX--
