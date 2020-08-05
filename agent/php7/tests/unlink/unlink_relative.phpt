--TEST--
hook unlink relative path
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('deleteFile', params => {
    assert(params.path == 'tmp_unlink_file')
    assert(params.realpath.endsWith('tmp_unlink_file'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
touch('tmp_unlink_file');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(unlink('tmp_unlink_file'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>