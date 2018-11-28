--TEST--
hook echo
--SKIPIF--
<?php
$conf = <<<CONF
xss_echo.action="log"
CONF;
include(__DIR__.'/../skipif.inc');
?>
--INI--
openrasp.root_dir=/tmp/openrasp
--GET--
a=<b>test</b>
--FILE--
<?php
echo $_GET['a'];
?>
--EXPECT--
<b>test</b>