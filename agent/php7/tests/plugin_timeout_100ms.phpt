--TEST--
plugin timeout
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'echo test')
    assert(params.stack[0].indexOf('exec') != -1)
    while(true);
    return block
})
EOF;
$conf = <<<CONF
plugin.timeout.millis: 100
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$start = round(microtime(true) * 1000);
exec('echo test');
$end = round(microtime(true) * 1000);
$interval = $end - $start;
if ($interval > 100 && $interval < 100 * 1.5)
{
    echo 'ok';
} 
else
{
    var_dump($interval);
    var_dump($start);
    var_dump($end);
}
?>
--EXPECT--
ok