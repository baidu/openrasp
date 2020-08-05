--TEST--
hook MongoCode::__construct
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mongo extension in PHP7.');
$plugin = <<<EOF
plugin.register('mongodb', params => {
    assert(params.query == 'function() { for(i=0;i<10;i++) {db.foo.update({z : i}, {z : x});}return x-1;}')
    assert(params.server == 'mongodb')
    assert(params.class == 'MongoCode')
    assert(params.method == '__construct')
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
$code = new MongoCode('function() { '.
    'for(i=0;i<10;i++) {'.
        'db.foo.update({z : i}, {z : x});'.
    '}'.
    'return x-1;'.
 '}', array("x" => 4));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>