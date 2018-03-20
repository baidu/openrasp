--TEST--
Check for command hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
?>
--FILE--
<?php
passthru('cd',$return_val);
echo 'passthru OK';
?>
--EXPECT--