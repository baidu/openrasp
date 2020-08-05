--TEST--
hook include
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('include', params => {
    assert(params.function == 'include')
    assert(params.path == 'test.inc')
    assert(params.url == 'test.inc')
    assert(params.realpath.endsWith('/test.inc'))
    return block
})
EOF;
$conf = <<<CONF
plugin.filter: false
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=force_to_cgi
--ENV--
return <<<END
DOCUMENT_ROOT=/tmp/openrasp
END;
--FILE--
<?php
include('test.inc');
echo 'no check';
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>