--TEST--
hook popen (webshell)
--SKIPIF--
<?php
$plugin = <<<EOF
RASP.algorithmConfig = {
    webshell_command: {
        name:   '算法2 - 拦截简单的 PHP 命令执行后门',
        action: 'block'
    }
}
EOF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=cd
--FILE--
<?php
popen($_GET['a'], 'r');
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>