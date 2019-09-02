--TEST--
block content html
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
block.content_html: "<p>OpenRASP Request ID: %request_id%</p>"
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
default_charset="UTF-8"
openrasp.root_dir=/tmp/openrasp
--ENV--
return <<<END
HTTP_ACCEPT=text/html;
END;
--FILE--
<?php
exec('echo test');
?>
--EXPECTHEADERS--
Content-type: text/html;charset=UTF-8
--EXPECTREGEX--
<p>OpenRASP Request ID: .*<\/p>