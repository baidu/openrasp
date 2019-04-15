--TEST--
hook assert eval
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: assert() is now a language construct and not a function in PHP7.');
$plugin = <<<EOF
plugin.register('eval', params => {
    console.log(params)
    assert(params.function = 'assert')
    assert(params.code == '2<1')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
assert('2<1');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>