--TEST--
json body parse application/xml
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
--ENV--
return <<<END
HTTP_CONTENT_TYPE=application/xml;
END;
--CGI--
--POST--
{"name":"JSON_BODY"}
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
exec('echo test');
passthru('tail -n 1 /tmp/openrasp/logs/plugin/plugin.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*{}.*