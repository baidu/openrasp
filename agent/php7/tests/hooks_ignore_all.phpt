--TEST--
hooks ignore all
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    assert(params.path.endsWith('/tmp/openrasp/tmpfile'))
    assert(params.realpath.endsWith('openrasp/tmpfile'))
    return block
})
plugin.register('writeFile', params => {
    assert(params.path.endsWith('/tmp/openrasp/tmpfile'))
    assert(params.realpath.endsWith('openrasp/tmpfile'))
    return block
})
EOF;
$conf = <<<CONF
hook.white:
  "*":
    - "all"
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
file_put_contents('/tmp/openrasp/tmpfile', 'test');
var_dump(file_get_contents('/tmp/openrasp/tmpfile'));
?>
--EXPECT--
string(4) "test"