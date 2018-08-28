--TEST--
request url
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', (params, context) => {
    assert(params.command == 'echo test')
    assert(context.url == 'https://rasp.baidu.com/index.php')
    return block
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--CGI--
--ENV--
return <<<END
REQUEST_SCHEME=https
HTTP_HOST=rasp.baidu.com
DOCUMENT_ROOT=/tmp/openrasp
REQUEST_URI=/index.php
END;
--FILE--
<?php
exec('echo test');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>