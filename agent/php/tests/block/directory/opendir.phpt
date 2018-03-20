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
// Open a known directory, and proceed to read its contents
if (is_dir($dir)) {
    if ($dh = opendir($dir)) {
        echo "opendir OK" . "\n";
        closedir($dh);
    }
}
?>
--EXPECT--