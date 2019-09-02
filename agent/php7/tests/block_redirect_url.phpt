--TEST--
block redirect url
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
block.redirect_url: "/block?request_id="
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
exec('echo test');
?>
--EXPECTHEADERS--
Location: /block?request_id=
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>