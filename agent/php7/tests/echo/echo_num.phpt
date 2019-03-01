--TEST--
hook echo num
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
     xss_echo: {
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
1=<b>test</b>
--FILE--
<?php
echo $_GET[1];
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>