--TEST--
hook rename
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    assert(params.source == '/tmp/openrasp/conf')
    assert(params.dest == '/tmp/openrasp/conf.php')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@rename('/tmp/openrasp/conf', '/tmp/openrasp/conf.php');
echo 'no check';
?>
--EXPECT--
no check