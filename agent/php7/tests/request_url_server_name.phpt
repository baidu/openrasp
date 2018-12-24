--TEST--
request url
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', (params, context) => {
    assert(params.command == 'echo test')
    assert(context.url == 'http://openrasp.test.com:8383/index.php')
    return block
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--ENV--
return <<<END
SERVER_NAME=openrasp.test.com
SERVER_PORT=8383
DOCUMENT_ROOT=/tmp/openrasp
REQUEST_URI=/index.php
END;
--FILE--
<?php
exec('echo test');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>