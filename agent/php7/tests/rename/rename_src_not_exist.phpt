--TEST--
hook rename
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    assert(params.source.endsWith('/tmp/non_exist_rename_source'))
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
@rename('/tmp/non_exist_rename_source', '/tmp/openrasp/tmpfile');
echo 'no check';
?>
--EXPECT--
no check