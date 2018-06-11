--TEST--
hooks ignore
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    assert(params.path == '/tmp/openrasp/tmpfile')
    assert(params.realpath.endsWith('openrasp/tmpfile'))
    return block
})
plugin.register('writeFile', params => {
    assert(params.path == '/tmp/openrasp/tmpfile')
    assert(params.realpath.endsWith('openrasp/tmpfile'))
    return block
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.hooks_ignore=readFile,writeFile
--FILE--
<?php
file_put_contents('/tmp/openrasp/tmpfile', 'test');
var_dump(file_get_contents('/tmp/openrasp/tmpfile'));
?>
--EXPECT--
string(4) "test"