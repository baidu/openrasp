--TEST--
hook eval eval
--SKIPIF--
<?php
$plugin = <<<EOF
plugin.register('eval', params => {
    console.log(params)
    assert(params.function = 'eval')
    assert(params.code == 'echo \'no check\';')
    return block
})
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=force_to_cgi
--FILE--
<?php
eval ("echo 'no check';");
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>