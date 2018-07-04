--TEST--
hook file_put_contents (url http)
--DESCRIPTION--
NO CHECK, cuz of HTTP wrapper does not support writeable connections
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('writeFile', params => {
    assert(params.path == 'http://www.example.com/')
    assert(params.realpath == 'http://www.example.com/')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@file_put_contents('http://www.example.com/', 'test');
echo 'no check'
?>
--EXPECT--
no check