--TEST--
Check for command hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
?>
--FILE--
<?php
exec('cd');
echo 'exec OK';
?>
--EXPECT--