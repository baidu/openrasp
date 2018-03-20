--TEST--
Check for readfile hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!stristr(PHP_OS, "Linux")) die("skip this test is Linux platforms only");
?>
--FILE--
<?php 
$names=file('/etc/hosts');
echo "file OK";
?>
--EXPECT--