--TEST--
hook rename
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    assert(params.source == '/tmp/openrasp/tmpfile')
    assert(params.dest == '/tmp/openrasp/tempdir/')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@rename('/tmp/openrasp/tmpfile', '/tmp/openrasp/tempdir/');
echo 'no check';
?>
--EXPECT--
no check