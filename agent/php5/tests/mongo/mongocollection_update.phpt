--TEST--
hook MongoCollection::update
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mongo extension in PHP7.');
$plugin = <<<EOF
plugin.register('mongodb', params => {
    assert(params.query == '{"likes":100}')
    assert(params.server == 'mongodb')
    assert(params.class == 'MongoCollection')
    assert(params.method == 'update')
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
$collection = $db->test;
$collection->update(array("likes" => 100), array("likes" => 200));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>