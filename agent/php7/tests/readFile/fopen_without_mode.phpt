--TEST--
hook fopen without mode
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    assert(params.path == '/tmp/openrasp/tmpfile')
    assert(params.realpath.endsWith('openrasp/tmpfile'))
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
var_dump(fopen('/tmp/openrasp/tmpfile'));
?>
--EXPECTREGEX--
.*bool\(false\).*