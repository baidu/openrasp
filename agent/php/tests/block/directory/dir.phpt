--TEST--
Check for directory hook
--SKIPIF--
<?php 
if (!extension_loaded("openrasp")) print "skip";
if (!stristr(PHP_OS, "Linux")) die("skip this test is Linux platforms only");
?>
--FILE--
<?php 
$d = dir("/home");
echo "Path: " . $d->path . "\n";
$d->close();
?>
--EXPECT--