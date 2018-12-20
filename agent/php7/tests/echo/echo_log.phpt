--TEST--
hook echo
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
     xss_echo: {
        name:   '算法1 - PHP: 禁止直接输出 GPC 参数',
        action: 'log'
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=<b>test</b>
--FILE--
<?php
echo $_GET['a'];
?>
--EXPECT--
<b>test</b>