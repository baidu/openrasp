--TEST--
log.maxburst=1
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('directory', params => {
    assert(params.path == '/bin/../bin')
    assert(params.realpath == '/bin')
    assert(params.stack[0].indexOf('scandir') != -1)
    return {action: 'log'}
})
plugin.register('command', params => {
    assert(params.command == 'cd')
    assert(params.stack[0].indexOf('system') != -1)
    return {action: 'log'}
})
EOF;
$conf = <<<CONF
log.maxburst: 1
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
scandir('/bin/../bin');
system('cd');
passthru('tail -n 1 /tmp/openrasp/logs/alarm/alarm.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*"attack_type":"directory".*