--TEST--
hook dir not exist
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('directory', params => block)
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
ini_set('display_errors', 'Off');
dir('/aaa/bbb/../../../bin');
echo 'ok';
?>
--EXPECT--
ok