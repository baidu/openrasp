--TEST--
alarm json
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'echo test')
    assert(params.stack[0].indexOf('exec') != -1)
    return {action: 'log'}
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--CGI--
--GET--
a[]=1&b=2
--ENV--
return <<<END
REQUEST_SCHEME=https
HTTP_HOST=rasp.baidu.com
DOCUMENT_ROOT=/tmp/openrasp
REQUEST_URI=/index.php
END;
--FILE--
<?php
include(__DIR__.'/timezone.inc');
header('Content-type: text/plain');
exec('echo test');
passthru('tail -n 1 /tmp/openrasp/logs/alarm/alarm.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*"body":"".*