--TEST--
block status code
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
openrasp.block_status_code=500
--FILE--
<?php
header('Content-type: text/plain');
exec('echo test');
?>
--EXPECTHEADERS--
Status: 500 Internal Server Error
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>