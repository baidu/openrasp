--TEST--
hook array_walk white
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
     webshell_callable: {
        name:   '算法4 - 拦截简单的 PHP array_map/walk/filter 后门',
        action: 'block',
        functions: [
            'exec'
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
$arr = array('echo ok');
array_walk($arr, "system");
?>
--EXPECTREGEX--
ok