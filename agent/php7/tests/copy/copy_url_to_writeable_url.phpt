--TEST--
hook copy
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('copy', params => {
    assert(params.source == 'http://www.example.com/')
    assert(params.dest.endsWith('openrasp/tmpfile'))
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
copy('http://www.example.com/', 'file:///tmp/openrasp/tmpfile');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>