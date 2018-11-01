--TEST--
hook rename
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    assert(params.source.endsWith('/tmp/openrasp/tmpfile'))
    assert(params.dest.endsWith('openrasp/tmpfile'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'test');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@rename('/aaa/bbb/../../../tmp/openrasp/tmpfile', '/tmp/openrasp/tmpfile');
echo 'no check';
?>
--EXPECT--
no check