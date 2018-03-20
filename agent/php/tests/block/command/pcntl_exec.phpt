--TEST--
Check for command hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
?>
--FILE--
<?php
$cmd = "/tmp/dummy/cmd";
$args = array("^_^");
pcntl_exec($cmd, $args);
echo "pcntl_exec OK";
?>
--EXPECT--