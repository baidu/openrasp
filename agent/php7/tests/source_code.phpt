--TEST--
source code
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'cd')
    assert(params.stack[0].indexOf('system') != -1)
    return {action: 'log'}
})
EOF;
$conf = <<<CONF
decompile.enable: true
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include(__DIR__.'/timezone.inc');
header('Content-type: text/plain');
system('cd');
passthru('tail -n 1 /tmp/openrasp/logs/alarm/alarm.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*"source_code":\["system\('cd'\);"\].*