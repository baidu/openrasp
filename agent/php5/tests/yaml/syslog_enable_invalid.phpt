--TEST--
syslog.enable invalid
--SKIPIF--
<?php
$conf = <<<CONF
syslog.enable: [1, 2, 3]
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
.*type should be boolen.*