--TEST--
hook copy ignore
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('copy', params => {
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
copy('/tmp/openrasp/tmpfile', '/tmp/openrasp/tmpfile');
echo 'no check';
?>
--EXPECTREGEX--
no check