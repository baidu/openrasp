--TEST--
hook file
--DESCRIPTION--
http://php.net/manual/en/function.file.php
local file with "://" 
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    assert(params.path.endsWith('/tmp/openrasp/http://tmpfile'))
    assert(params.realpath.endsWith('/tmp/openrasp/http:/tmpfile'))
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
if (!is_dir('/tmp/openrasp/http:')) {
    mkdir('/tmp/openrasp/http:', 0777, true);
}
file_put_contents('/tmp/openrasp/http:/tmpfile', 'temp');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(file('/tmp/openrasp/http://tmpfile'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>