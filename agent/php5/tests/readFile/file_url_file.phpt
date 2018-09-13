--TEST--
hook file
--DESCRIPTION--
http://php.net/manual/en/function.file.php
url file://
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    assert(params.path == 'file:///tmp/openrasp/tmpfile')
    assert(params.realpath.endsWith('/tmp/openrasp/tmpfile'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(file('file:///tmp/openrasp/tmpfile'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>