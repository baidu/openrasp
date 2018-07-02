--TEST--
hook mysql_query
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mysql extension in PHP7.');
$plugin = <<<EOF
plugin.register('sql', params => {
    assert(params.query == 'SELECT a FROM b')
    assert(params.server == 'mysql')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mysql")) die("Skipped: mysql extension required.");
@$con = mysql_connect('127.0.0.1', 'root');
if (!$con) die("Skipped: can not connect to MySQL " . mysql_error());
mysql_close($con);
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.enforce_policy=Off
--FILE--
<?php
@$con = mysql_connect('127.0.0.1', 'root');
mysql_query("SELECT a FROM b");
mysql_close($con);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>