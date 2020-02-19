--TEST--
hook scandir open_basedir plugin.filter off
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('directory', params => {
    assert(params.path == '/bin/../bin')
    assert(params.realpath == '/bin')
    assert(params.stack[0].indexOf('scandir') != -1)
    return block
})
EOF;
$conf = <<<CONF
plugin.filter: false
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
open_basedir=.:/etc:/tmp
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
var_dump(scandir('/bin/../bin'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>