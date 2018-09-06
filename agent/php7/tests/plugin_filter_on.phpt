--TEST--
plugin_filter on
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    console.log(params)
    assert(params.path.endsWith('/tmp/openrasp/do_not_exits'))
    assert(params.realpath.endsWith('/tmp/openrasp/do_not_exits'))
    return block
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.plugin_filter=on
--FILE--
<?php
ini_set('display_errors', 'off');
file_get_contents('/tmp/openrasp/do_not_exits');
// file('/tmp/openrasp/do_not_exits');
// fopen('/tmp/openrasp/do_not_exits', 'r');
// readFile('/tmp/openrasp/do_not_exits');
echo ok;
?>
--EXPECT--
ok