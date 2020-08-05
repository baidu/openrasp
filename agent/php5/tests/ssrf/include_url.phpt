--TEST--
hook include ssrf
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('ssrf', params => {
    assert(params.url == 'http://www.example.com/')
    assert(params.function == 'include')
    assert(params.hostname == 'www.example.com')
    assert(Array.isArray(params.ip))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=force_to_cgi
--ENV--
return <<<END
DOCUMENT_ROOT=/
END;
--FILE--
<?php
include('http://www.example.com/');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>