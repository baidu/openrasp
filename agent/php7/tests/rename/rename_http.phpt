--TEST--
hook rename
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    assert(params.source == 'http://www.example.com/')
    assert(params.dest == 'http://www.example.com/')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@rename('http://www.example.com/', 'http://www.example.com/');
echo 'no check';
?>
--EXPECT--
no check