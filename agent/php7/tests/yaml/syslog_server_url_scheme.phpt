--TEST--
syslog server url scheme
--SKIPIF--
<?php
$conf = <<<CONF
syslog.enable: true
syslog.url: "http://127.0.0.1:514"
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php

?>
--EXPECTREGEX--
.*Invalid url scheme in syslog server address.*