--TEST--
hook scandir open_basedir plugin.filter on
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('directory', params => {
    assert(params.path == '/bin/../bin')
    assert(params.realpath == '/bin')
    assert(params.stack[0].indexOf('scandir') != -1)
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
open_basedir=.:/etc:/tmp
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(scandir('/bin/../bin'));
?>
--EXPECTREGEX--
.*restriction in effect.*