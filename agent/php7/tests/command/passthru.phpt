--TEST--
hook passthru (with stack)
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'cd')
    assert(params.stack[0].indexOf('passthru') != -1)
    assert(params.stack[1].indexOf('test_stack_1') != -1)
    assert(params.stack[2].indexOf('test_stack_2') != -1)
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
function test_stack_1()
{
    passthru('cd');
}
function test_stack_2()
{
    test_stack_1();
}
test_stack_2();
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>