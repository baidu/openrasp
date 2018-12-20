--TEST--
hook system (webshell) action log
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
    webshell_command: {
        name:   '算法2 - 拦截简单的 PHP 命令执行后门',
        action: 'log'
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=echo ok
--FILE--
<?php
system($_GET['a']);
?>
--EXPECTREGEX--
ok