--TEST--
hook opendir
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('directory', params => {
    assert(params.path == 'http://www.example.com/')
    assert(params.realpath == 'http://www.example.com/')
    assert(params.stack[0].indexOf('opendir') != -1)
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@opendir('http://www.example.com/');
echo 'no check';
?>
--EXPECT--
no check