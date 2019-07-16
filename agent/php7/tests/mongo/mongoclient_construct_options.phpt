--TEST--
hook MongoClient::__construct options
--SKIPIF--
<?php
if (PHP_MAJOR_VERSION >= 7) die('Skipped: no mongo extension in PHP7.');
$conf = <<<CONF
security.weak_passwords:
  - ""
  - "root"
  - "123"
  - "123456"
  - "a123456"
  - "123456a"
  - "111111"
  - "123123"
  - "admin"
  - "user"
  - "mysql"
CONF;
include(__DIR__.'/../skipif.inc');
if (!extension_loaded("mongo")) die("Skipped: mongo extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$m = new MongoClient("mongodb://localhost:27017/test", array('username'=>'openrasp','password'=>'rasp#2019'));
?>
--EXPECT--
