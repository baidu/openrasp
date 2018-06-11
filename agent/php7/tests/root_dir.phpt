--TEST--
root dir
--SKIPIF--
<?php
if (!extension_loaded("openrasp")) die("Skipped: openrasp extension required.");
?>
--INI--
openrasp.root_dir=openrasp
--FILE--
<?php

?>
--EXPECTREGEX--
.*root_dir.*