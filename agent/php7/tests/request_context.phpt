--TEST--
request context
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', (params, context) => {
    assert(params.command == 'echo test')
    assert(params.stack[0].indexOf('exec') != -1)
    let str = JSON.stringify(context)
    assert(/"requestId":"\w+"/.test(str))
    assert(str.includes('"language":"php"'))
    assert(str.includes('"server":"PHP"'))
    assert(str.includes('"appBasePath":"/tmp/openrasp"'))
    assert(str.includes('"remoteAddr":"127.0.0.1"'))
    assert(str.includes('"protocol":"http"'))
    assert(str.includes('"method":"post"'))
    assert(str.includes('"querystring":"a[]=1&b=2"'))
    assert(str.includes('"path":"/index.php"'))
    assert(str.includes('"parameter":{"a":["1","2"],"b":["2"],"c":["1"],"d":["2"]}'))
    assert(str.includes('"host":"rasp.baidu.com"'))
    assert(str.includes('"content-length":"13"'))
    assert(str.includes('"content-type":"application/x-www-form-urlencoded"'))
    assert(str.includes('"url":"http://rasp.baidu.com/index.php"'))
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