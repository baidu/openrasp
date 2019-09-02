--TEST--
block content xml
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
block.content_xml: "<?xml version=\"1.0\"?><doc><error>true</error><reason>blocked by OpenRASP</reason><request_id>%request_id%</request_id></doc>"
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
default_charset="UTF-8"
openrasp.root_dir=/tmp/openrasp
--ENV--
return <<<END
HTTP_ACCEPT=text/xml;
END;
--FILE--
<?php
exec('echo test');
?>
--EXPECTHEADERS--
Content-type: text/xml;charset=UTF-8
--EXPECTREGEX--
<\?xml version="1.0"\?><doc><error>true<\/error><reason>blocked by OpenRASP<\/reason><request_id>.*<\/request_id><\/doc>