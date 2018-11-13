--TEST--
hook array_walk white
--SKIPIF--
<?php
$conf = <<<CONF
callable.blacklist=["exec"]
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