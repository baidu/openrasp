--TEST--
hook unlink non exist
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('deleteFile', params => {
    assert(params.path == '/tmp/openrasp/non-exist-file')
    assert(params.realpath == '/tmp/openrasp/non-exist-file')
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
@unlink('/tmp/openrasp/non-exist-file');
echo 'no check';
?>
--EXPECTREGEX--
no check