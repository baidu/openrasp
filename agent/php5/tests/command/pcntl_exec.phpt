--TEST--
hook pcntl_exec
--SKIPIF--
<?php
if (!function_exists("pcntl_exec")) die("Skipped: pcntl is disabled.");
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'cd / &')
    assert(params.stack[0].indexOf('pcntl_exec') != -1)
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
pcntl_exec('cd', ['/', '&']);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>