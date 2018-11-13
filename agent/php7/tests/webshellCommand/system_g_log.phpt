--TEST--
hook system (webshell) action log
--SKIPIF--
<?php
$conf = <<<CONF
webshell_command.action="log"
CONF;
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