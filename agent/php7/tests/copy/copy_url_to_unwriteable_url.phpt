--TEST--
hook copy
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('copy', params => {
    assert(params.source == 'http://www.example.com/')
    assert(params.dest == 'http://www.example.com/')
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
@copy('http://www.example.com/', 'http://www.example.com/');
echo 'no check';
?>
--EXPECTREGEX--
no check