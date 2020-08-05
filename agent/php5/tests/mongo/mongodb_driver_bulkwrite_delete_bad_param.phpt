--TEST--
hook MongoDB\Driver\BulkWrite::delete bad param
--SKIPIF--
<?php
if (PHP_VERSION_ID < 50500) die('Skipped: not supported (version < 5.5.0)');
$plugin = <<<EOF
plugin.register('mongodb', params => {
    assert(params.query == '{"likes":100}')
    assert(params.server == 'mongodb')
    assert(params.class.endsWith('Bulkwrite'))
    assert(params.method == 'delete')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mongodb")) die("Skipped: mongodb extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$manager = new MongoDB\Driver\Manager("mongodb://openrasp:rasp#2019@localhost:27017/test");
$bulk = new MongoDB\Driver\BulkWrite;
$bulk->delete(null);
?>
--EXPECTREGEX--
Warning: MongoDB\\Driver\\BulkWrite::delete\(\) expects .*