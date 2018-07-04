--TEST--
hook SQLite3::querySingle
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('sql', params => {
    assert(params.query == 'SELECT a FROM b')
    assert(params.server == 'sqlite')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("sqlite3")) die("Skipped: sqlite3 extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.enforce_policy=Off
--FILE--
<?php
$db = new SQLite3('test.db');
$results = $db->querySingle('SELECT a FROM b');
$db->close();
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>