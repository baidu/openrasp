--TEST--
hook file_put_contents (url ftp)
--DESCRIPTION--
url ftp
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('writeFile', params => {
    assert(params.path == 'FTP://name:password@ftp.example.com/test.html')
    assert(params.realpath == 'FTP://name:password@ftp.example.com/test.html')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
@file_put_contents('FTP://name:password@ftp.example.com/test.html', 'test');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>