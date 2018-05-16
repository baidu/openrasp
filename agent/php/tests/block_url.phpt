--TEST--
block url
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'echo test')
    assert(params.stack[0].endsWith('exec'))
    return block
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.block_url=http://block_url.com
--FILE--
<?php
exec('echo test');
?>
--EXPECTREGEX--
.*block_url.*