--TEST--
hook dir relative
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('directory', params => {
    assert(params.path == 'tests')
    assert(params.realpath.endsWith('tests'))
    assert(params.stack[0].indexOf('dir') != -1)
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
dir("tests");
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>