--TEST--
plugin.maxstack invalid
--SKIPIF--
<?php
$conf = <<<CONF
plugin.maxstack: [1, 2, 3]
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
include(__DIR__.'/../timezone.inc');
passthru('tail -n 1 /tmp/openrasp/logs/rasp/rasp.log.'.date("Y-m-d"));
?>
--EXPECTREGEX--
.*type should be number.*