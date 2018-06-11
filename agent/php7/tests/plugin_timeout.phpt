--TEST--
plugin timeout
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'echo test')
    assert(params.stack[0].endsWith('exec'))
    while(true);
    return block
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.timeout_ms=2000
--FILE--
<?php
$start = time();
exec('echo test');
$end = time();
if ($end - $start > 1)
{
    echo 'ok';
} 
else
{
    var_dump($start);
    var_dump($end);
}
?>
--EXPECT--
ok