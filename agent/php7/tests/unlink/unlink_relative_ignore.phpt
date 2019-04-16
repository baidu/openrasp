--TEST--
hook unlink relative path ignore
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('deleteFile', params => {
    assert(params.path == 'tmp_unlink_file')
    assert(params.realpath.endsWith('tmp_unlink_file'))
    return ignore
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
--EXPECT--
bool(true)