--TEST--
block content json
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
block.content_json: "{\"error\":true, \"reason\": \"blocked by OpenRASP\", \"request_id\": \"%request_id%\"}"
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
default_charset="UTF-8"
openrasp.root_dir=/tmp/openrasp
--ENV--
return <<<END
HTTP_ACCEPT=application/json;
END;
--FILE--
<?php
exec('echo test');
?>
--EXPECTHEADERS--
Content-type: application/json
--EXPECTREGEX--
{"error":true, "reason": "blocked by OpenRASP", "request_id": ".*"}