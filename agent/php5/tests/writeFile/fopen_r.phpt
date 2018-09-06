--TEST--
hook fopen (r)
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('writeFile', params => {
    assert(params.path.endsWith('/tmp/openrasp/tmpfile'))
    assert(params.realpath.endsWith('/tmp/openrasp/tmpfile'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
fopen('/tmp/openrasp/tmpfile', 'r');
?>
ok
--EXPECT--
ok