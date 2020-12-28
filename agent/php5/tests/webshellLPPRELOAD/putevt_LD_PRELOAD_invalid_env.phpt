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
putenv('LD_PRELOAD')
?>
ok
--EXPECTREGEX--
ok