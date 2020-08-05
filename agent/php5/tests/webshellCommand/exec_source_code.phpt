--TEST--
source code exec (webshell)
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
$conf = <<<CONF
decompile.enable: true
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=cd
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
exec($_GET['a']);
passthru('tail -n 1 /tmp/openrasp/logs/alarm/alarm.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*"source_code":\["exec\(\$_GET\['a'\]\);"\].*