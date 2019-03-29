--TEST--
hook SQLite3::querySingle bad param
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('sql', params => {
    assert(params.query == 'SELECT a FROM b')
    assert(params.server == 'sqlite')
    return block
})
EOF;
$conf = <<<CONF
security.enforce_policy: false
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("sqlite3")) die("Skipped: sqlite3 extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$db = new SQLite3('test.db');
$results = $db->querySingle(array());
$db->close();
?>
--EXPECTREGEX--
Warning: SQLite3::querySingle\(\) expects.*