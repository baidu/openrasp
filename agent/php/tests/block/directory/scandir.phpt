--TEST--
Check for directory hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!stristr(PHP_OS, "Linux")) die("skip this test is Linux platforms only");
?>
--FILE--
<?php
$dir = "/home";
$files1 = scandir($dir);
echo "scandir OK" . "\n";
?>
--EXPECT--