--TEST--
hook eval (webshell) action log
--SKIPIF--
<?php
$conf = <<<CONF
webshell_eval.action="log"
CONF;
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