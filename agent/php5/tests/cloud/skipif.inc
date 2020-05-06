<?php
exec(__DIR__.'/resources/start.sh >/tmp/openrasp/cloud.log 2>&1');
sleep(1);
$fp = fsockopen("127.0.0.1", 8383, $errno, $errstr, 30);
if (!$fp) {
  die("Skipped: cannot connect to cloud.");
}
?>