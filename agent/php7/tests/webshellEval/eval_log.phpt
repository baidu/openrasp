--TEST--
hook eval (webshell) action log
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
    webshell_eval: {
        name:   '算法1 - 拦截简单的 PHP 中国菜刀后门',
        action: 'log'
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=$val=1;
--FILE--
<?php
$val=0;
eval($_GET['a']);
echo $val;
?>
--EXPECTREGEX--
1