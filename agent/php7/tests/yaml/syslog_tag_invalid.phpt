--TEST--
syslog.tag invalid
--SKIPIF--
<?php
$conf = <<<CONF
syslog.tag: "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
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
.*should be number and alphabeta, and length between 1 and 32.*