--TEST--
hook rename
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    assert(params.source == 'ftp://name:password@ftp.example.com/test.html')
    assert(params.dest == 'phar://some.phar')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@rename('ftp://name:password@ftp.example.com/test.html', 'phar://some.phar');
echo 'no check';
?>
--EXPECTREGEX--
no check