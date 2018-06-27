--TEST--
hook file
--DESCRIPTION--
http://php.net/manual/en/function.file.php
url http://www.example.com/
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    assert(params.path == 'http://www.example.com/')
    assert(params.realpath == 'http://www.example.com/')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(file('http://www.example.com/'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>