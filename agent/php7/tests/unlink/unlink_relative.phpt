--TEST--
hook unlink relative path
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('deleteFile', params => {
    assert(params.path == 'tmpfile')
    assert(params.realpath.endsWith('tmpfile'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
touch('tmpfile');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(unlink('tmpfile'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>