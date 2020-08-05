--TEST--
yaml invalid
--SKIPIF--
<?php
$conf = <<<CONF
map: name: John
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
.*Fail to parse config, cuz of message.*