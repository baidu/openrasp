--TEST--
hook MongoDB::execute bad param
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mongo extension in PHP7.');
$plugin = <<<EOF
plugin.register('mongodb', params => {
    assert(params.query == '"foo";')
    assert(params.server == 'mongodb')
    assert(params.class == 'MongoDB')
    assert(params.method == 'execute')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mongo")) die("Skipped: mongo extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$m = new MongoClient("mongodb://openrasp:rasp#2019@localhost:27017/test");
$db = $m->test;
$db->execute();
?>
--EXPECTREGEX--
.*Warning: MongoDB::execute\(\) expects .*