--TEST--
hook echo_filter
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
     xss_echo: {
        filter_regex: "^[a-zA-Z]{1,10}$",
        name:   '算法1 - PHP: 禁止直接输出 GPC 参数',
        action: 'block'
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=abc123
--FILE--
<?php
echo $_GET['a'];
?>
--EXPECTREGEX--
abc123