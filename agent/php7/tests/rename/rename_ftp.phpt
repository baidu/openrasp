--TEST--
hook rename
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    assert(params.source == 'ftp://name:password@ftp.example.com/src.html')
    assert(params.dest == 'ftp://name:password@ftp.example.com/dest.html')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
rename('ftp://name:password@ftp.example.com/src.html', 'ftp://name:password@ftp.example.com/dest.html');
echo 'no check';
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>