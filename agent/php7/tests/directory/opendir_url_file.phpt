--TEST--
hook opendir
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('directory', params => {
    console.log(params)
    assert(params.path == 'file:///tmp/openrasp')
    assert(params.realpath.endsWith('/tmp/openrasp'))
    assert(params.stack[0].indexOf('opendir') != -1)
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(opendir('file:///tmp/openrasp'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>