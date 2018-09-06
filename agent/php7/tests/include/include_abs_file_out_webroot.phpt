--TEST--
hook include
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('include', params => {
    assert(params.function = 'include')
    assert(params.path == '/etc/passwd')
    assert(params.url.endsWith('/etc/passwd'))
    assert(params.realpath.endsWith('/etc/passwd'))
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
DOCUMENT_ROOT=/tmp/openrasp
END;
--FILE--
<?php
include('/etc/passwd');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>