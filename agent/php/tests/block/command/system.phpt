--TEST--
Check for command hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
?>
--FILE--
<?php
system('cd',$return_val);
echo 'system OK';
?>
--EXPECT--