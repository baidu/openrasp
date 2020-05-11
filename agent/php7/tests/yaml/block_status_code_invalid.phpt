--TEST--
block.status_code invalid
--SKIPIF--
<?php
$conf = <<<CONF
block.status_code: -1
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
.*the value shoule be >= 0.*