--TEST--
plugin.filter false
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('readFile', params => {
    console.log(params)
    assert(params.path.endsWith('/tmp/openrasp/do_not_exits'))
    assert(params.realpath.endsWith('/tmp/openrasp/do_not_exits'))
    return block
})
EOF;
$conf = <<<CONF
plugin.filter: false
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
ini_set('display_errors', 'off');
var_dump(file_get_contents('/tmp/openrasp/do_not_exits'));
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>