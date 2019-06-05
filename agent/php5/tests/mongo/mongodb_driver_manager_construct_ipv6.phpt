--TEST--
hook MongoDB\Driver\Manager::__construct ipv6
--SKIPIF--
<?php
if (PHP_VERSION_ID < 50500) die('Skipped: not supported (version < 5.5.0)');
$conf = <<<CONF
security.enforce_policy: true
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
if (!extension_loaded("mongodb")) die("Skipped: mongodb extension required.");
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
<?php
$manager = new MongoDB\Driver\Manager("mongodb://[::1]:27015");
?>
--EXPECTREGEX--
<\/script><script>location.href="http[s]?:\/\/.*?request_id=[0-9a-f]{32}"<\/script>