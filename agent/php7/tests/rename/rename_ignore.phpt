--TEST--
hook rename ignore
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    return ignore
})
EOF;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'test');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
rename('/tmp/openrasp/tmpfile', '/tmp/openrasp/tmpfile');
echo 'no check';
?>
--EXPECTREGEX--
no check