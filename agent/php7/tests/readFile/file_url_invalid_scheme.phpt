--TEST--
hook file
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    assert(params.path == 'dummy://www.example.com/')
    assert(params.realpath == 'dummy://www.example.com/')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@file('dummy://www.example.com/');
echo 'no check';
?>
--EXPECT--
no check