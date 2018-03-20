--TEST--
Check for command hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
?>
--FILE--
<?php
$handle = popen('/tmp/dummy/cmd', 'r');
pclose($handle);
echo 'popen OK';
?>
--EXPECT--