--TEST--
hook rename plugin.filter true
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    assert(params.source.endsWith('include'))
    assert(params.dest.endsWith('include'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'test');
?>
--INI--
open_basedir=.:/etc:/tmp
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
rename('/usr/include', '/tmp/include');
?>
--EXPECTREGEX--
.*restriction in effect.*