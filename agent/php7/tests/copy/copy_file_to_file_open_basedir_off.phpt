--TEST--
hook copy plugin.filter false
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
plugin.register('copy', params => {
    plugin.log(params)
    assert(params.source.endsWith('/usr/include'))
    assert(params.dest.endsWith('/tmp/include'))
    return block
})
EOF;
$conf = <<<CONF
plugin.filter: false
CONF;
include(__DIR__.'/../skipif.inc');
file_put_contents('/tmp/openrasp/tmpfile', 'test');
?>
--INI--
open_basedir=.:/etc:/tmp
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
copy('/usr/include', '/tmp/include');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>