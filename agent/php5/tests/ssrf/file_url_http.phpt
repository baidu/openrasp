--TEST--
hook file
--DESCRIPTION--
http://php.net/manual/en/function.file.php
url http://www.example.com/
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('ssrf', params => {
    assert(params.url == 'http://www.example.com/')
    assert(params.function == 'file')
    assert(params.hostname == 'www.example.com')
    assert(Array.isArray(params.ip))
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