--TEST--
hook file_put_contents (exist)
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('writeFile', params => {
    assert(params.path.endsWith('/tmp/openrasp/tmpfile'))
    assert(params.realpath.endsWith('/tmp/openrasp/tmpfile'))
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
file_put_contents('/tmp/openrasp/tmpfile', 'test', FILE_APPEND);
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>