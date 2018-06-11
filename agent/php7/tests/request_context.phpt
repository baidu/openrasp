--TEST--
request context
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', (params, context) => {
    assert(params.command == 'echo test')
    assert(params.stack[0].endsWith('exec'))
    for (let k in context) {
        assert(!!context[k])
    }
    assert(String.fromCharCode.apply(this, new Uint8Array(context.body)) == 'c=1&d=2&a[]=2')
    return block
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a[]=1&b=2
--POST--
c=1&d=2&a[]=2
--ENV--
return <<<END
REQUEST_SCHEME=http
HTTP_HOST=rasp.baidu.com
REMOTE_ADDR=127.0.0.1
DOCUMENT_ROOT=/tmp/openrasp
REQUEST_URI=/index.php
END;
--FILE--
<?php
exec('echo test');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>