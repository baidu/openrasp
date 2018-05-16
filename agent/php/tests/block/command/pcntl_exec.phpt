--TEST--
Check for command hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!extension_loaded("pcntl")) print "skip";
?>
--FILE--
<?php
$cmd = "/tmp/dummy/cmd";
$args = array("^_^");
pcntl_exec($cmd, $args);
echo "pcntl_exec OK";
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>