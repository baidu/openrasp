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
openrasp.root_dir=/tmp/openrasp
openrasp.block_content_json="{\"error\":true, \"reason\": \"Request blocked by OpenRASP\", \"request_id\": \"OPENRASP_REQUEST_ID\"}"
--ENV--
return <<<END
HTTP_ACCEPT=application/json;
END;
--FILE--
<?php
exec('echo test');
?>
--EXPECTREGEX--
{"error":true, "reason": "Request blocked by OpenRASP", "request_id": ".*"}