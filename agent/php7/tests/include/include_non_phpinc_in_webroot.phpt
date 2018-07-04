--TEST--
hook include
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('include', params => {
    assert(params.function = 'include')
    assert(params.path.endsWith('/../include.txt'))
    assert(params.url.endsWith('/../include.txt'))
    assert(params.realpath.endsWith('/include.txt'))
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
include(__DIR__.'/../include.txt');
echo 'no check';
?>
--EXPECT--
openrasp
no check