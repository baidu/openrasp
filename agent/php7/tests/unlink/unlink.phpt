--TEST--
hook unlink
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('deleteFile', params => {
    assert(params.path == '/tmp/openrasp/tmpfile')
    assert(params.realpath == '/tmp/openrasp/tmpfile')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'test');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(unlink('/tmp/openrasp/tmpfile'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>