--TEST--
alarm log algorithm
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'echo test')
    assert(params.stack[0].indexOf('exec') != -1)

        var request_config = {
        "method":       "get",
        "maxRedirects": 0,
        "headers": {
            "content-type": "application/json"
        },        
    }
        request_config.url = 'http://127.0.0.1:9797/index.php'



    // console.log("send to:", algorithmConfig['iast']['fuzz_server'])
    RASP.request_async(request_config)

    return {action: 'log', algorithm: '666'}
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include(__DIR__.'/timezone.inc');
header('Content-type: text/plain');
exec('echo test');
passthru('tail -n 1 /tmp/openrasp/logs/alarm/alarm.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*algorithm.*666.*