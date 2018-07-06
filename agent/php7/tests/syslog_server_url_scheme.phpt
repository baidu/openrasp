--TEST--
syslog server utl scheme
--SKIPIF--
<?php
if (!extension_loaded("openrasp")) die("Skipped: openrasp extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
openrasp.syslog_alarm_enable=On
openrasp.syslog_server_address=http://127.0.0.1:514
--FILE--
<?php

?>
--EXPECTREGEX--
.*Invalid url scheme in syslog server address.*