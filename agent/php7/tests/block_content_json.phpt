--TEST--
block content json
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('command', params => {
    assert(params.command == 'echo test')
    assert(params.stack[0].endsWith('exec'))
    return block
})
EOF;
include(__DIR__.'/skipif.inc');
?>
--INI--
default_charset="UTF-8"
openrasp.root_dir=/tmp/openrasp
openrasp.block_content_json="{\"error\":true, \"reason\": \"Request blocked by OpenRASP\", \"request_id\": \"%request_id%\"}"
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
{"error":true, "reason": "Request blocked by OpenRASP", "request_id": ".*"}