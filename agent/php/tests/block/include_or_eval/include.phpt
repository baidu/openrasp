--TEST--
Check for include hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
?>
--FILE--
<?php
include('/../../../../../../../../../../etc/hosts');
echo 'include OK';
?>
--EXPECT--