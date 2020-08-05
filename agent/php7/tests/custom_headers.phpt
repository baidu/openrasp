--TEST--
custom headers
--SKIPIF--
<?php
$conf = <<<CONF
inject.custom_headers:
  custom: headers
  AAAAAA: BBBBBBB
CONF;
include(__DIR__.'/skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--FILE--
ok
--EXPECTHEADERS--
custom: headers
AAAAAA: BBBBBBB
--EXPECTREGEX--
ok