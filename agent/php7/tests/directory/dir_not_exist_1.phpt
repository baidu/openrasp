--TEST--
hook dir not exist
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('directory', params => {
    assert(params.path == 'openrasp_test_not_exist')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
ini_set('display_errors', 'Off');
dir('openrasp_test_not_exist');
echo 'ok';
?>
--EXPECT--
ok