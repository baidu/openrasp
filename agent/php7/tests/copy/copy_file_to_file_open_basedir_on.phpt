--TEST--
hook copy plugin.filter true
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('copy', params => {
    plugin.log(params)
    assert(params.source.endsWith('/usr/include'))
    assert(params.dest.endsWith('/tmp/include'))
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
copy('/usr/include', '/tmp/include');
?>
--EXPECTREGEX--
.*restriction in effect.*