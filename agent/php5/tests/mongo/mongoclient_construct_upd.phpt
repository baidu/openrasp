--TEST--
hook MongoClient::__construct username password database
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mongo extension in PHP7.');
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mongo")) die("Skipped: mongo extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$m = new MongoClient("mongodb://openrasp:rasp#2019@localhost:27017/test");
?>
--EXPECT--
