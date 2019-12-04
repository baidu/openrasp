--TEST--
response header
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
inject.custom_headers:
  X-Protected-By: OpenRASP
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$headers = headers_list();
foreach ($headers as $header) {
    if (strstr($header, 'X-Request-ID') == 0) {
        header('X-Request-ID: 001ae04bbf142185000147562aecaebe', true);
    }
}
exec('echo test');
?>
ok
--EXPECTHEADERS--
X-Request-ID: 001ae04bbf142185000147562aecaebe
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>