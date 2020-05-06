--TEST--
syslog
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
syslog.enable: true
syslog.tag: "OpenRASP"
syslog.url: "tcp://127.0.0.1:514"
syslog.facility: 1
syslog.connection_timeout: 50
syslog.read_timeout: 10
syslog.reconnect_interval: 300
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
system('cd');
?>
--EXPECT--