--TEST--
json body parse application/json
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', (params, context) => {
    plugin.log(context.json)
    return {action: 'ignore'}
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--CGI--
--POST_RAW--
Content-Type: application/json
{"name":"JSON_BODY"}
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
exec('echo test');
passthru('tail -n 1 /tmp/openrasp/logs/plugin/plugin.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*{ name: 'JSON_BODY' }.*