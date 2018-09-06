--TEST--
hook file_put_contents (not exist)
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('writeFile', params => {
    assert(params.path.endsWith('/tmp/openrasp/not_exist'))
    assert(params.realpath.endsWith('/tmp/openrasp/not_exist'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(file_put_contents('/tmp/openrasp/not_exist', 'test'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>