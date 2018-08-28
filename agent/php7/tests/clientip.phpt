--TEST--
clientip
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', (params, context) => {
    assert(params.command == 'echo test')
    return {action: 'log'}
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.clientip_header=clientip
--CGI--
--ENV--
return <<<END
REQUEST_SCHEME=http
HTTP_HOST=rasp.baidu.com
HTTP_CLIENTIP=1.1.1.1, 2.2.2.2, 3.3.3.3
REMOTE_ADDR=127.0.0.1
DOCUMENT_ROOT=/tmp/openrasp
REQUEST_URI=/index.php
END;
--FILE--
<?php
include(__DIR__.'/timezone.inc');
exec('echo test');
passthru('tail -n 1 /tmp/openrasp/logs/alarm/alarm.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*"attack_source":"3.3.3.3".*