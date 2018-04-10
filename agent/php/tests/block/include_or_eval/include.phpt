--TEST--
Check for include hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
?>
--FILE--
<?php
include('/../header.rasp');
echo 'include OK';
?>
--EXPECT--