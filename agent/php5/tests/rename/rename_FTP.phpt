--TEST--
hook rename
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('rename', params => {
    assert(params.source == 'FTP://name:password@ftp.example.com/test.html')
    assert(params.dest == 'FTP://name:password@ftp.example.com/test.html')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
rename('FTP://name:password@ftp.example.com/test.html', 'FTP://name:password@ftp.example.com/test.html');
echo 'no check';
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>