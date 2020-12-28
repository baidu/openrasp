--TEST--
hook putenv (webshell)
--SKIPIF--
<?php
$dir = __DIR__;
$plugin = <<<EOF
RASP.algorithmConfig = {
    webshell_ld_preload: {
        action: 'block',
        env: [
            'LD_PRELOAD',
            'LD_AUDIT',
            'GCONV_PATH'
        ]
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
putenv('LD_AUDIT=2333')
?>
ok
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>