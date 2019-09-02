--TEST--
block status code
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'echo test')
    assert(params.stack[0].indexOf('exec') != -1)
    return block
})
EOF;
$conf = <<<CONF
block.status_code: 500
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
header('Content-type: text/plain');
exec('echo test');
?>
--EXPECTHEADERS--
Status: 500 Internal Server Error
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>