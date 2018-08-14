--TEST--
block content html
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
openrasp.block_content_html="<p>OpenRASP Request ID: OPENRASP_REQUEST_ID</p>"
--ENV--
return <<<END
HTTP_ACCEPT=text/html;
END;
--FILE--
<?php
exec('echo test');
?>
--EXPECTREGEX--
<p>OpenRASP Request ID: .*<\/p>