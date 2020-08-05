--TEST--
root dir mkdir
--SKIPIF--
<?php
if (!extension_loaded("openrasp")) die("Skipped: openrasp extension required.");
include(__DIR__.'/utils.inc');
cleanup_directpry("/tmp/openrasp");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php

?>
--EXPECT--
