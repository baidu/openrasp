--TEST--
Check for writefile hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!stristr(PHP_OS, "Linux")) die("skip this test is Linux platforms only");
?>
--FILE--
<?php 
$handle = fopen("/etc/environment", "a+");
echo "fopen OK";
?>
--EXPECT--