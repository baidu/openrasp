--TEST--
hook file_put_contents (url http)
--DESCRIPTION--
url ftp
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
//nothing
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