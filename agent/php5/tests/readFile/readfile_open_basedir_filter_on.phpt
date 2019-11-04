--TEST--
hook readfile plugin.filter on
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    assert(params.path.endsWith('/usr'))
    assert(params.realpath.endsWith('/usr'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
open_basedir=.:/etc:/tmp
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(readfile('/usr'));
?>
--EXPECTREGEX--
.*restriction in effect.*