--TEST--
block content xml
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
openrasp.block_content_json="<?xml version=\"1.0\"?><doc><error>true</error><reason>Request blocked by OpenRASP</reason><request_id>OPENRASP_REQUEST_ID</request_id></doc>"
--ENV--
return <<<END
HTTP_ACCEPT=text/xml;
END;
--FILE--
<?php
exec('echo test');
?>
--EXPECTREGEX--
<\?xml version="1.0"\?><doc><error>true<\/error><reason>Request blocked by OpenRASP<\/reason><request_id>.*<\/request_id><\/doc>