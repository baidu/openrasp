--TEST--
hook array_walk action log
--SKIPIF--
<?php
$conf = <<<CONF
webshell_callable.blacklist=["system", "exec"]
webshell_callable.action="log"
CONF;
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