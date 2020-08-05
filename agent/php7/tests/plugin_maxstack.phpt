--TEST--
plugin maxstack
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'echo test')
    assert(params.stack[0].indexOf('exec') != -1)
    assert(params.stack.length <= 10)
    return block
})
EOF;
$conf = <<<CONF
plugin.maxstack: 10
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
function test($deep)
{
    if ($deep < 20)
    {
        test($deep + 1);
    }
    else
    {
        exec('echo test');
    }
}
test(0);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>