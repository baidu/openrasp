--TEST--
hook include
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('include', params => {
    assert(params.function = 'include')
    assert(params.path.endsWith('/tmp/openrasp/http://tmpfile'))
    assert(params.url.endsWith('/tmp/openrasp/http://tmpfile'))
    assert(params.realpath.endsWith('/tmp/openrasp/http:/tmpfile'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
if (!is_dir('/tmp/openrasp/http:')) {
    mkdir('/tmp/openrasp/http:', 0777, true);
}
file_put_contents('/tmp/openrasp/http:/tmpfile', 'temp');
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
include('/tmp/openrasp/http://tmpfile');
?>
--EXPECT--
temp